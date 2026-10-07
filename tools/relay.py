"""UDP relay for the sango online mode (lib/include/net/wire.h).

Usage: python tools/relay.py [port]

Players send their pose ~10 Hz. Ten times a second the relay tells each of
them where everybody else in the same zone is. Players that stay silent for
a few seconds are dropped.

On join, the relay gives each player the lowest free outfit (the look of its
avatar, 0..3) and, when the player did not ask for a name, the first unused
name of NAMES.
"""
import asyncio
import struct
import sys
import time

MAGIC = 0x4753
VERSION = 2
HEADER = struct.Struct("<HBBHH")          # magic, version, type, slot, sequence
POSE = struct.Struct("<HBBfff")           # zone, flags, heading, x, y, z
WELCOME = struct.Struct("<HB16s")         # slot, outfit, name
NAME_LENGTH = 16
OUTFIT_COUNT = 4
MAX_WORLD_ENTRIES = 24

NAMES = ["Alice", "Bob", "Clara", "Dylan", "Emma", "Felix", "Gaelle", "Hugo"]

JOIN, WELCOME_TYPE, POSE_TYPE, WORLD, LEAVE, KEEP_ALIVE = 1, 2, 3, 4, 5, 6

TICK_SECONDS = 0.1
TIMEOUT_SECONDS = 15.0


def pack_name(name):
    return name.encode("utf-8")[:NAME_LENGTH].ljust(NAME_LENGTH, b"\0")


class Player:
    def __init__(self, slot, name, outfit, address):
        self.slot = slot
        self.name = name
        self.outfit = outfit
        self.address = address
        self.pose = (0, 0, 0, 0.0, 0.0, 0.0)
        self.last_seen = time.monotonic()


class Relay(asyncio.DatagramProtocol):
    def __init__(self):
        self.players = {}  # address -> Player
        self.next_slot = 1
        self.sequence = 0
        self.transport = None

    def connection_made(self, transport):
        self.transport = transport

    def send(self, address, kind, slot, body=b""):
        self.sequence = (self.sequence + 1) & 0xFFFF
        header = HEADER.pack(MAGIC, VERSION, kind, slot, self.sequence)
        self.transport.sendto(header + body, address)

    def allocate_slot(self):
        used = {p.slot for p in self.players.values()}
        while self.next_slot in used or self.next_slot == 0:
            self.next_slot = (self.next_slot % 0xFFFF) + 1
        slot = self.next_slot
        self.next_slot = (self.next_slot % 0xFFFF) + 1
        return slot

    def allocate_outfit(self):
        used = {p.outfit for p in self.players.values()}
        for outfit in range(OUTFIT_COUNT):
            if outfit not in used:
                return outfit
        return len(self.players) % OUTFIT_COUNT  # full: looks repeat

    def allocate_name(self, requested):
        if requested:
            return requested
        used = {p.name for p in self.players.values()}
        for name in NAMES:
            if name not in used:
                return name
        return f"Player{self.next_slot}"

    def datagram_received(self, data, address):
        if len(data) < HEADER.size:
            return
        magic, version, kind, _slot, _seq = HEADER.unpack_from(data)
        if magic != MAGIC or version != VERSION:
            return
        body = data[HEADER.size:]
        player = self.players.get(address)

        if kind == JOIN:
            if player is None:
                requested = body[:NAME_LENGTH].split(b"\0", 1)[0].decode("utf-8", "replace")
                player = Player(self.allocate_slot(), self.allocate_name(requested),
                                self.allocate_outfit(), address)
                self.players[address] = player
                print(f"+ #{player.slot} {player.name!r} outfit {player.outfit} from {address}")
            player.last_seen = time.monotonic()
            self.send(address, WELCOME_TYPE, player.slot,
                      WELCOME.pack(player.slot, player.outfit, pack_name(player.name)))
            return

        if player is None:
            # Unknown sender (relay restarted, or timed out): slot 0 = rejoin.
            self.send(address, WELCOME_TYPE, 0, WELCOME.pack(0, 0, b""))
            return

        player.last_seen = time.monotonic()
        if kind == POSE_TYPE and len(body) >= POSE.size:
            player.pose = POSE.unpack_from(body)
        elif kind == LEAVE:
            self.drop(address, "left")

    def drop(self, address, reason):
        player = self.players.pop(address, None)
        if player:
            print(f"- #{player.slot} {player.name!r} {reason}")

    async def tick(self):
        while True:
            await asyncio.sleep(TICK_SECONDS)
            now = time.monotonic()
            for address in [a for a, p in self.players.items()
                            if now - p.last_seen > TIMEOUT_SECONDS]:
                self.drop(address, "timed out")

            for address, me in self.players.items():
                zone = me.pose[0]
                others = [p for p in self.players.values()
                          if p is not me and p.pose[0] == zone]
                others = others[:MAX_WORLD_ENTRIES]
                body = bytes([len(others)])
                for other in others:
                    body += (struct.pack("<H", other.slot) + POSE.pack(*other.pose)
                             + bytes([other.outfit]) + pack_name(other.name))
                self.send(address, WORLD, 0, body)


async def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 5000
    loop = asyncio.get_running_loop()
    relay = Relay()
    await loop.create_datagram_endpoint(lambda: relay, local_addr=("0.0.0.0", port))
    print(f"relay listening on udp/{port}")
    await relay.tick()


if __name__ == "__main__":
    asyncio.run(main())

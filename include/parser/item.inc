/*
 * Copyright (C) 2026  David Darras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

static const char* ITEM_NAMES[] = {
    "None", // 0
    "Master Ball", // 1
    "Ultra Ball", // 2
    "Great Ball", // 3
    "Poké Ball", // 4
    "Safari Ball", // 5
    "Net Ball", // 6
    "Dive Ball", // 7
    "Nest Ball", // 8
    "Repeat Ball", // 9
    "Timer Ball", // 10
    "Luxury Ball", // 11
    "Premier Ball", // 12
    "Dusk Ball", // 13
    "Heal Ball", // 14
    "Quick Ball", // 15
    "Cherish Ball", // 16
    "Potion", // 17
    "Antidote", // 18
    "Burn Heal", // 19
    "Ice Heal", // 20
    "Awakening", // 21
    "Paralyze Heal", // 22
    "Full Restore", // 23
    "Max Potion", // 24
    "Hyper Potion", // 25
    "Super Potion", // 26
    "Full Heal", // 27
    "Revive", // 28
    "Max Revive", // 29
    "Fresh Water", // 30
    "Soda Pop", // 31
    "Lemonade", // 32
    "Moomoo Milk", // 33
    "Energy Powder", // 34
    "Energy Root", // 35
    "Heal Powder", // 36
    "Revival Herb", // 37
    "Ether", // 38
    "Max Ether", // 39
    "Elixir", // 40
    "Max Elixir", // 41
    "Lava Cookie", // 42
    "Berry Juice", // 43
    "Sacred Ash", // 44
    "HP Up", // 45
    "Protein", // 46
    "Iron", // 47
    "Carbos", // 48
    "Calcium", // 49
    "Rare Candy", // 50
    "PP Up", // 51
    "Zinc", // 52
    "PP Max", // 53
    "Old Gateau", // 54
    "Guard Spec.", // 55
    "Dire Hit", // 56
    "X Attack", // 57
    "X Defense", // 58
    "X Speed", // 59
    "X Accuracy", // 60
    "X Sp. Atk", // 61
    "X Sp. Def", // 62
    "Poké Doll", // 63
    "Fluffy Tail", // 64
    "Blue Flute", // 65
    "Yellow Flute", // 66
    "Red Flute", // 67
    "Black Flute", // 68
    "White Flute", // 69
    "Shoal Salt", // 70
    "Shoal Shell", // 71
    "Red Shard", // 72
    "Blue Shard", // 73
    "Yellow Shard", // 74
    "Green Shard", // 75
    "Super Repel", // 76
    "Max Repel", // 77
    "Escape Rope", // 78
    "Repel", // 79
    "Sun Stone", // 80
    "Moon Stone", // 81
    "Fire Stone", // 82
    "Thunder Stone", // 83
    "Water Stone", // 84
    "Leaf Stone", // 85
    "Tiny Mushroom", // 86
    "Big Mushroom", // 87
    "Pearl", // 88
    "Big Pearl", // 89
    "Stardust", // 90
    "Star Piece", // 91
    "Nugget", // 92
    "Heart Scale", // 93
    "Honey", // 94
    "Growth Mulch", // 95
    "Damp Mulch", // 96
    "Stable Mulch", // 97
    "Gooey Mulch", // 98
    "Root Fossil", // 99
    "Claw Fossil", // 100
    "Helix Fossil", // 101
    "Dome Fossil", // 102
    "Old Amber", // 103
    "Armor Fossil", // 104
    "Skull Fossil", // 105
    "Rare Bone", // 106
    "Shiny Stone", // 107
    "Dusk Stone", // 108
    "Dawn Stone", // 109
    "Oval Stone", // 110
    "Odd Keystone", // 111
    "Griseous Orb", // 112
    "unknown", // 113
    "unknown", // 114
    "unknown", // 115
    "Douse Drive", // 116
    "Shock Drive", // 117
    "Burn Drive", // 118
    "Chill Drive", // 119
    "unknown", // 120
    "unknown", // 121
    "unknown", // 122
    "unknown", // 123
    "unknown", // 124
    "unknown", // 125
    "unknown", // 126
    "unknown", // 127
    "unknown", // 128
    "unknown", // 129
    "unknown", // 130
    "unknown", // 131
    "unknown", // 132
    "unknown", // 133
    "Sweet Heart", // 134
    "Adamant Orb", // 135
    "Lustrous Orb", // 136
    "Greet Mail", // 137
    "Favored Mail", // 138
    "RSVP Mail", // 139
    "Thanks Mail", // 140
    "Inquiry Mail", // 141
    "Like Mail", // 142
    "Reply Mail", // 143
    "Bridge Mail S", // 144
    "Bridge Mail D", // 145
    "Bridge Mail T", // 146
    "Bridge Mail V", // 147
    "Bridge Mail M", // 148
    "Cheri Berry", // 149
    "Chesto Berry", // 150
    "Pecha Berry", // 151
    "Rawst Berry", // 152
    "Aspear Berry", // 153
    "Leppa Berry", // 154
    "Oran Berry", // 155
    "Persim Berry", // 156
    "Lum Berry", // 157
    "Sitrus Berry", // 158
    "Figy Berry", // 159
    "Wiki Berry", // 160
    "Mago Berry", // 161
    "Aguav Berry", // 162
    "Iapapa Berry", // 163
    "Razz Berry", // 164
    "Bluk Berry", // 165
    "Nanab Berry", // 166
    "Wepear Berry", // 167
    "Pinap Berry", // 168
    "Pomeg Berry", // 169
    "Kelpsy Berry", // 170
    "Qualot Berry", // 171
    "Hondew Berry", // 172
    "Grepa Berry", // 173
    "Tamato Berry", // 174
    "Cornn Berry", // 175
    "Magost Berry", // 176
    "Rabuta Berry", // 177
    "Nomel Berry", // 178
    "Spelon Berry", // 179
    "Pamtre Berry", // 180
    "Watmel Berry", // 181
    "Durin Berry", // 182
    "Belue Berry", // 183
    "Occa Berry", // 184
    "Passho Berry", // 185
    "Wacan Berry", // 186
    "Rindo Berry", // 187
    "Yache Berry", // 188
    "Chople Berry", // 189
    "Kebia Berry", // 190
    "Shuca Berry", // 191
    "Coba Berry", // 192
    "Payapa Berry", // 193
    "Tanga Berry", // 194
    "Charti Berry", // 195
    "Kasib Berry", // 196
    "Haban Berry", // 197
    "Colbur Berry", // 198
    "Babiri Berry", // 199
    "Chilan Berry", // 200
    "Liechi Berry", // 201
    "Ganlon Berry", // 202
    "Salac Berry", // 203
    "Petaya Berry", // 204
    "Apicot Berry", // 205
    "Lansat Berry", // 206
    "Starf Berry", // 207
    "Enigma Berry", // 208
    "Micle Berry", // 209
    "Custap Berry", // 210
    "Jaboca Berry", // 211
    "Rowap Berry", // 212
    "Bright Powder", // 213
    "White Herb", // 214
    "Macho Brace", // 215
    "Exp. Share", // 216
    "Quick Claw", // 217
    "Soothe Bell", // 218
    "Mental Herb", // 219
    "Choice Band", // 220
    "King's Rock", // 221
    "Silver Powder", // 222
    "Amulet Coin", // 223
    "Cleanse Tag", // 224
    "Soul Dew", // 225
    "Deep Sea Tooth", // 226
    "Deep Sea Scale", // 227
    "Smoke Ball", // 228
    "Everstone", // 229
    "Focus Band", // 230
    "Lucky Egg", // 231
    "Scope Lens", // 232
    "Metal Coat", // 233
    "Leftovers", // 234
    "Dragon Scale", // 235
    "Light Ball", // 236
    "Soft Sand", // 237
    "Hard Stone", // 238
    "Miracle Seed", // 239
    "Black Glasses", // 240
    "Black Belt", // 241
    "Magnet", // 242
    "Mystic Water", // 243
    "Sharp Beak", // 244
    "Poison Barb", // 245
    "Never-Melt Ice", // 246
    "Spell Tag", // 247
    "Twisted Spoon", // 248
    "Charcoal", // 249
    "Dragon Fang", // 250
    "Silk Scarf", // 251
    "Up-Grade", // 252
    "Shell Bell", // 253
    "Sea Incense", // 254
    "Lax Incense", // 255
    "Lucky Punch", // 256
    "Metal Powder", // 257
    "Thick Club", // 258
    "Stick", // 259
    "Red Scarf", // 260
    "Blue Scarf", // 261
    "Pink Scarf", // 262
    "Green Scarf", // 263
    "Yellow Scarf", // 264
    "Wide Lens", // 265
    "Muscle Band", // 266
    "Wise Glasses", // 267
    "Expert Belt", // 268
    "Light Clay", // 269
    "Life Orb", // 270
    "Power Herb", // 271
    "Toxic Orb", // 272
    "Flame Orb", // 273
    "Quick Powder", // 274
    "Focus Sash", // 275
    "Zoom Lens", // 276
    "Metronome", // 277
    "Iron Ball", // 278
    "Lagging Tail", // 279
    "Destiny Knot", // 280
    "Black Sludge", // 281
    "Icy Rock", // 282
    "Smooth Rock", // 283
    "Heat Rock", // 284
    "Damp Rock", // 285
    "Grip Claw", // 286
    "Choice Scarf", // 287
    "Sticky Barb", // 288
    "Power Bracer", // 289
    "Power Belt", // 290
    "Power Lens", // 291
    "Power Band", // 292
    "Power Anklet", // 293
    "Power Weight", // 294
    "Shed Shell", // 295
    "Big Root", // 296
    "Choice Specs", // 297
    "Flame Plate", // 298
    "Splash Plate", // 299
    "Zap Plate", // 300
    "Meadow Plate", // 301
    "Icicle Plate", // 302
    "Fist Plate", // 303
    "Toxic Plate", // 304
    "Earth Plate", // 305
    "Sky Plate", // 306
    "Mind Plate", // 307
    "Insect Plate", // 308
    "Stone Plate", // 309
    "Spooky Plate", // 310
    "Draco Plate", // 311
    "Dread Plate", // 312
    "Iron Plate", // 313
    "Odd Incense", // 314
    "Rock Incense", // 315
    "Full Incense", // 316
    "Wave Incense", // 317
    "Rose Incense", // 318
    "Luck Incense", // 319
    "Pure Incense", // 320
    "Protector", // 321
    "Electirizer", // 322
    "Magmarizer", // 323
    "Dubious Disc", // 324
    "Reaper Cloth", // 325
    "Razor Claw", // 326
    "Razor Fang", // 327
    "TM01", // 328
    "TM02", // 329
    "TM03", // 330
    "TM04", // 331
    "TM05", // 332
    "TM06", // 333
    "TM07", // 334
    "TM08", // 335
    "TM09", // 336
    "TM10", // 337
    "TM11", // 338
    "TM12", // 339
    "TM13", // 340
    "TM14", // 341
    "TM15", // 342
    "TM16", // 343
    "TM17", // 344
    "TM18", // 345
    "TM19", // 346
    "TM20", // 347
    "TM21", // 348
    "TM22", // 349
    "TM23", // 350
    "TM24", // 351
    "TM25", // 352
    "TM26", // 353
    "TM27", // 354
    "TM28", // 355
    "TM29", // 356
    "TM30", // 357
    "TM31", // 358
    "TM32", // 359
    "TM33", // 360
    "TM34", // 361
    "TM35", // 362
    "TM36", // 363
    "TM37", // 364
    "TM38", // 365
    "TM39", // 366
    "TM40", // 367
    "TM41", // 368
    "TM42", // 369
    "TM43", // 370
    "TM44", // 371
    "TM45", // 372
    "TM46", // 373
    "TM47", // 374
    "TM48", // 375
    "TM49", // 376
    "TM50", // 377
    "TM51", // 378
    "TM52", // 379
    "TM53", // 380
    "TM54", // 381
    "TM55", // 382
    "TM56", // 383
    "TM57", // 384
    "TM58", // 385
    "TM59", // 386
    "TM60", // 387
    "TM61", // 388
    "TM62", // 389
    "TM63", // 390
    "TM64", // 391
    "TM65", // 392
    "TM66", // 393
    "TM67", // 394
    "TM68", // 395
    "TM69", // 396
    "TM70", // 397
    "TM71", // 398
    "TM72", // 399
    "TM73", // 400
    "TM74", // 401
    "TM75", // 402
    "TM76", // 403
    "TM77", // 404
    "TM78", // 405
    "TM79", // 406
    "TM80", // 407
    "TM81", // 408
    "TM82", // 409
    "TM83", // 410
    "TM84", // 411
    "TM85", // 412
    "TM86", // 413
    "TM87", // 414
    "TM88", // 415
    "TM89", // 416
    "TM90", // 417
    "TM91", // 418
    "TM92", // 419
    "HM01", // 420
    "HM02", // 421
    "HM03", // 422
    "HM04", // 423
    "HM05", // 424
    "HM06", // 425
    "unknown", // 426
    "unknown", // 427
    "Explorer Kit", // 428
    "Loot Sack", // 429
    "Rule Book", // 430
    "Poké Radar", // 431
    "Point Card", // 432
    "Journal", // 433
    "Seal Case", // 434
    "Fashion Case", // 435
    "Seal Bag", // 436
    "Pal Pad", // 437
    "Works Key", // 438
    "Old Charm", // 439
    "Galactic Key", // 440
    "Red Chain", // 441
    "Town Map", // 442
    "Vs. Seeker", // 443
    "Coin Case", // 444
    "Old Rod", // 445
    "Good Rod", // 446
    "Super Rod", // 447
    "Sprayduck", // 448
    "Poffin Case", // 449
    "Bike", // 450
    "Suite Key", // 451
    "Oak's Letter", // 452
    "Lunar Wing", // 453
    "Member Card", // 454
    "Azure Flute", // 455
    "S.S. Ticket", // 456
    "Contest Pass", // 457
    "Magma Stone", // 458
    "Parcel", // 459
    "Coupon 1", // 460
    "Coupon 2", // 461
    "Coupon 3", // 462
    "Storage Key", // 463
    "Secret Potion", // 464
    "Vs. Recorder", // 465
    "Gracidea", // 466
    "Secret Key", // 467
    "Apricorn Box", // 468
    "Unown Report", // 469
    "Berry Pots", // 470
    "Dowsing Machine", // 471
    "Blue Card", // 472
    "Slowpoke Tail", // 473
    "Clear Bell", // 474
    "Card Key", // 475
    "Basement Key", // 476
    "Squirt Bottle", // 477
    "Red Scale", // 478
    "Lost Item", // 479
    "Pass", // 480
    "Machine Part", // 481
    "Silver Wing", // 482
    "Rainbow Wing", // 483
    "Mystery Egg", // 484
    "Red Apricorn", // 485
    "Blue Apricorn", // 486
    "Yellow Apricorn", // 487
    "Green Apricorn", // 488
    "Pink Apricorn", // 489
    "White Apricorn", // 490
    "Black Apricorn", // 491
    "Fast Ball", // 492
    "Level Ball", // 493
    "Lure Ball", // 494
    "Heavy Ball", // 495
    "Love Ball", // 496
    "Friend Ball", // 497
    "Moon Ball", // 498
    "Sport Ball", // 499
    "Park Ball", // 500
    "Photo Album", // 501
    "GB Sounds", // 502
    "Tidal Bell", // 503
    "Rage Candy Bar", // 504
    "Data Card 01", // 505
    "Data Card 02", // 506
    "Data Card 03", // 507
    "Data Card 04", // 508
    "Data Card 05", // 509
    "Data Card 06", // 510
    "Data Card 07", // 511
    "Data Card 08", // 512
    "Data Card 09", // 513
    "Data Card 10", // 514
    "Data Card 11", // 515
    "Data Card 12", // 516
    "Data Card 13", // 517
    "Data Card 14", // 518
    "Data Card 15", // 519
    "Data Card 16", // 520
    "Data Card 17", // 521
    "Data Card 18", // 522
    "Data Card 19", // 523
    "Data Card 20", // 524
    "Data Card 21", // 525
    "Data Card 22", // 526
    "Data Card 23", // 527
    "Data Card 24", // 528
    "Data Card 25", // 529
    "Data Card 26", // 530
    "Data Card 27", // 531
    "Jade Orb", // 532
    "Lock Capsule", // 533
    "Red Orb", // 534
    "Blue Orb", // 535
    "Enigma Stone", // 536
    "Prism Scale", // 537
    "Eviolite", // 538
    "Float Stone", // 539
    "Rocky Helmet", // 540
    "Air Balloon", // 541
    "Red Card", // 542
    "Ring Target", // 543
    "Binding Band", // 544
    "Absorb Bulb", // 545
    "Cell Battery", // 546
    "Eject Button", // 547
    "Fire Gem", // 548
    "Water Gem", // 549
    "Electric Gem", // 550
    "Grass Gem", // 551
    "Ice Gem", // 552
    "Fighting Gem", // 553
    "Poison Gem", // 554
    "Ground Gem", // 555
    "Flying Gem", // 556
    "Psychic Gem", // 557
    "Bug Gem", // 558
    "Rock Gem", // 559
    "Ghost Gem", // 560
    "Dragon Gem", // 561
    "Dark Gem", // 562
    "Steel Gem", // 563
    "Normal Gem", // 564
    "Health Wing", // 565
    "Muscle Wing", // 566
    "Resist Wing", // 567
    "Genius Wing", // 568
    "Clever Wing", // 569
    "Swift Wing", // 570
    "Pretty Wing", // 571
    "Cover Fossil", // 572
    "Plume Fossil", // 573
    "Liberty Pass", // 574
    "Pass Orb", // 575
    "Dream Ball", // 576
    "Poké Toy", // 577
    "Prop Case", // 578
    "Dragon Skull", // 579
    "Balm Mushroom", // 580
    "Big Nugget", // 581
    "Pearl String", // 582
    "Comet Shard", // 583
    "Relic Copper", // 584
    "Relic Silver", // 585
    "Relic Gold", // 586
    "Relic Vase", // 587
    "Relic Band", // 588
    "Relic Statue", // 589
    "Relic Crown", // 590
    "Casteliacone", // 591
    "Dire Hit 2", // 592
    "X Speed 2", // 593
    "X Sp. Atk 2", // 594
    "X Sp. Def 2", // 595
    "X Defense 2", // 596
    "X Attack 2", // 597
    "X Accuracy 2", // 598
    "X Speed 3", // 599
    "X Sp. Atk 3", // 600
    "X Sp. Def 3", // 601
    "X Defense 3", // 602
    "X Attack 3", // 603
    "X Accuracy 3", // 604
    "X Speed 6", // 605
    "X Sp. Atk 6", // 606
    "X Sp. Def 6", // 607
    "X Defense 6", // 608
    "X Attack 6", // 609
    "X Accuracy 6", // 610
    "Ability Urge", // 611
    "Item Drop", // 612
    "Item Urge", // 613
    "Reset Urge", // 614
    "Dire Hit 3", // 615
    "Light Stone", // 616
    "Dark Stone", // 617
    "TM93", // 618
    "TM94", // 619
    "TM95", // 620
    "Xtransceiver", // 621
    "God Stone", // 622
    "Gram 1", // 623
    "Gram 2", // 624
    "Gram 3", // 625
    "Xtransceiver", // 626
    "Medal Box", // 627
    "DNA Splicers", // 628
    "DNA Splicers", // 629
    "Permit", // 630
    "Oval Charm", // 631
    "Shiny Charm", // 632
    "Plasma Card", // 633
    "Grubby Hanky", // 634
    "Colress Machine", // 635
    "Dropped Item", // 636
    "Dropped Item", // 637
    "Reveal Glass", // 638
    "Weakness Policy", // 639
    "Assault Vest", // 640
    "Holo Caster", // 641
    "Prof's Letter", // 642
    "Roller Skates", // 643
    "Pixie Plate", // 644
    "Ability Capsule", // 645
    "Whipped Dream", // 646
    "Sachet", // 647
    "Luminous Moss", // 648
    "Snowball", // 649
    "Safety Goggles", // 650
    "Poké Flute", // 651
    "Rich Mulch", // 652
    "Surprise Mulch", // 653
    "Boost Mulch", // 654
    "Amaze Mulch", // 655
    "Gengarite", // 656
    "Gardevoirite", // 657
    "Ampharosite", // 658
    "Venusaurite", // 659
    "Charizardite X", // 660
    "Blastoisinite", // 661
    "Mewtwonite X", // 662
    "Mewtwonite Y", // 663
    "Blazikenite", // 664
    "Medichamite", // 665
    "Houndoominite", // 666
    "Aggronite", // 667
    "Banettite", // 668
    "Tyranitarite", // 669
    "Scizorite", // 670
    "Pinsirite", // 671
    "Aerodactylite", // 672
    "Lucarionite", // 673
    "Abomasite", // 674
    "Kangaskhanite", // 675
    "Gyaradosite", // 676
    "Absolite", // 677
    "Charizardite Y", // 678
    "Alakazite", // 679
    "Heracronite", // 680
    "Mawilite", // 681
    "Manectite", // 682
    "Garchompite", // 683
    "Latiasite", // 684
    "Latiosite", // 685
    "Roseli Berry", // 686
    "Kee Berry", // 687
    "Maranga Berry", // 688
    "Sprinklotad", // 689
    "TM96", // 690
    "TM97", // 691
    "TM98", // 692
    "TM99", // 693
    "TM100", // 694
    "Power Plant Pass", // 695
    "Mega Ring", // 696
    "Intriguing Stone", // 697
    "Common Stone", // 698
    "Discount Coupon", // 699
    "Elevator Key", // 700
    "TMV Pass", // 701
    "Honor of Kalos", // 702
    "Adventure Rules", // 703
    "Strange Souvenir", // 704
    "Lens Case", // 705
    "Travel Trunk", // 706
    "Travel Trunk", // 707
    "Lumiose Galette", // 708
    "Shalour Sable", // 709
    "Jaw Fossil", // 710
    "Sail Fossil", // 711
    "Looker Ticket", // 712
    "Bike", // 713
    "Holo Caster", // 714
    "Fairy Gem", // 715
    "Mega Charm", // 716
    "Mega Glove", // 717
    "Mach Bike", // 718
    "Acro Bike", // 719
    "Wailmer Pail", // 720
    "Devon Parts", // 721
    "Soot Sack", // 722
    "Basement Key", // 723
    "Pokéblock Kit", // 724
    "Letter", // 725
    "Eon Ticket", // 726
    "Scanner", // 727
    "Go-Goggles", // 728
    "Meteorite", // 729
    "Key to Room 1", // 730
    "Key to Room 2", // 731
    "Key to Room 4", // 732
    "Key to Room 6", // 733
    "Storage Key", // 734
    "Devon Scope", // 735
    "S.S. Ticket", // 736
    "HM07", // 737
    "Devon Scuba Gear", // 738
    "Contest Costume", // 739
    "Contest Costume", // 740
    "Magma Suit", // 741
    "Aqua Suit", // 742
    "Pair of Tickets", // 743
    "Mega Bracelet", // 744
    "Mega Pendant", // 745
    "Mega Glasses", // 746
    "Mega Anchor", // 747
    "Mega Stickpin", // 748
    "Mega Tiara", // 749
    "Mega Anklet", // 750
    "Meteorite", // 751
    "Swampertite", // 752
    "Sceptilite", // 753
    "Sablenite", // 754
    "Altarianite", // 755
    "Galladite", // 756
    "Audinite", // 757
    "Metagrossite", // 758
    "Sharpedonite", // 759
    "Slowbronite", // 760
    "Steelixite", // 761
    "Pidgeotite", // 762
    "Glalitite", // 763
    "Diancite", // 764
    "Prison Bottle", // 765
    "Mega Cuff", // 766
    "Cameruptite", // 767
    "Lopunnite", // 768
    "Salamencite", // 769
    "Beedrillite", // 770
    "Meteorite", // 771
    "Meteorite", // 772
    "Key Stone", // 773
    "Meteorite Shard", // 774
    "Eon Flute" // 775
};

static const size_t ITEM_COUNT = SIZE(ITEM_NAMES);

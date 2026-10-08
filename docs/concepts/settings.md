# Settings

A **setting** is a variable of a feature that changes how the feature works.
Your product sets it in its entry point. The player can also change it in the
menu.

The plugin does not save the settings: they return to their default values
at each start of the game.

## Where the settings are

Each feature with settings has a `...Settings` structure. The feature inherits it:

```cpp
// lib/include/pokemon/patch/shiny.h
struct ShinySettings {
  ShinyRate rate = ShinyRate::kOff;
};

struct Shiny : public ShinySettings {
  MAKE_SINGLETON(Shiny)
  // The rest of the feature: hooks, callbacks...
};
```

The `...Settings` structure contains only the values. The callbacks and the
work data of the feature are in the feature itself.

## Use a setting

In your product, set a setting like a normal variable:

```cpp
pokemon::Shiny::GetInstance().rate = pokemon::ShinyRate::k1_16;
```

In a menu page, give the setting to `app.Add`:

```cpp
app.Add("Shiny rate", pokemon::Shiny::GetInstance().rate);
```

## Add settings to a feature

1. Make a structure `MyFeatureSettings` with default values.
2. Add `static_assert(std::is_standard_layout<MyFeatureSettings>::value, "...")`.
3. Make the feature inherit `MyFeatureSettings`.

Keep the `...Settings` structures simple (numbers, booleans, enum classes,
no pointers): a future version of the plugin can then save them to a file.

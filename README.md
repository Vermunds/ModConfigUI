# ModConfigUI

In-game settings menu library using multiple backends for my Skyrim mods.
Settings are shown in both SKSE Menu Framework and Fuzz's Legally Intelligible Core Kit (F.L.I.C.K/F.U.C.K), whichever is installed. It also has built-in support for localizations.
This is primarily made for my needs, thus it's opinionated and doesn't support every feature the backends have.
As this library is mostly boilerplate, large portions were AI-generated.

## Usage

Link the `ModConfigUI` target, then call `Install` once in `kPostLoad`:

```cpp
static constexpr ModConfigUI::Page PAGES[] = { { "$MyMod_General", DrawGeneralPage } };
ModConfigUI::Install(modInfo, PAGES, RestoreDefaults);
```

A built-in "Mod Info" page is added in front of your settings pages.

## Translations

Strings are loaded from `Interface/Translations/<PluginName>_<LANGUAGE>.txt` (UTF-16 LE). Every mod's file must contain all `$ModConfigUI_*` keys. A missing key shows up raw in the menu.

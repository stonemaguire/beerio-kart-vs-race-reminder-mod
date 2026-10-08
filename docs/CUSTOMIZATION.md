# Customization

There are two independent sets of user-facing text: the two Local VS setting labels compiled into `subsdk9`, and the popup message stored in `message.sarc`.

## Change the two VS setting labels

Edit:

```text
include/beerio/text_config.hpp
```

The defaults are:

```cpp
inline constexpr char16_t FrequencyLabel[] =
    u"Beerio Race Frequency";

inline constexpr char16_t FrequencyStartLabel[] =
    u"Beerio Race Start";
```

Keep the `u"..."` UTF-16 string literal prefix. After changing either label, rebuild `subsdk9` with `make` and reinstall the files in `exefs`.

The displayed frequency values (`Every 1 Race`, `Every 2 Races`, and so on) are generated in `source/beerio/custom_selector.cpp`. If you make those strings substantially longer, also increase `CustomSelector::valueText` in `include/beerio/custom_selector.hpp`.

## Change the popup text

Edit the UTF-8 text file:

```text
config/popup.txt
```

Blank lines and manual line breaks are preserved. Unicode is encoded into the MSBT as UTF-16LE, although whether a character displays correctly still depends on the game's font.

The mod uses MSBT label **60000** (`0xEA60`). Do not change that label in `source/beerio/reminder_dialog.cpp` unless you also change the resource-generation tool.

### Build a customized `message.sarc`

Obtain your own **Mario Kart 8 Deluxe 4.0.0 US English** `message.sarc`, then run:

```bash
python tools/build_message_sarc.py \
  --input /path/to/original/message.sarc \
  --text config/popup.txt \
  --output romfs/UI/USen/message.sarc
```

The script can patch either an original archive or an archive it previously patched. It adds or updates label `60000` in `Menu.msbt` while preserving the rest of the SARC contents.

Install the generated file at:

```text
atmosphere/contents/0100152000022000/romfs/UI/USen/message.sarc
```

A full game relaunch is recommended after replacing the file.

## Change the reminder schedule behavior

The schedule is implemented in:

```text
source/beerio/reminder_settings.cpp
```

The current rule is:

```text
(upcomingRace - start) % frequency == 0
```

Only change this if you want behavior beyond renaming/rewording the mod.

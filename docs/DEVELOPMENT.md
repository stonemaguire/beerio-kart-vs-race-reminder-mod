# Development

## Requirements

- devkitPro with **devkitA64**
- Git
- GNU Make
- Python 3.10+
- The `external/mk8dx` submodule

`ftputil` is only required for optional FTP deployment.

## Clone

```bash
git clone --recursive <repository-url>
cd mk8dx-beerio-reminder-vs-race
```

For an existing clone:

```bash
git submodule update --init --recursive
```

## Environment

A typical devkitPro shell uses:

```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITA64=$DEVKITPRO/devkitA64
export PATH="$DEVKITA64/bin:$DEVKITPRO/tools/bin:$PATH"
```

Adjust the paths if your devkitPro installation is elsewhere.

## Build

```bash
make clean
make
```

Build products are written to `deploy/` and are intentionally ignored by Git.

Expected outputs include:

```text
deploy/main.npdm
deploy/subsdk9
```

## Build the popup resource

The source repository intentionally does not track extracted game archives. Generate the modified US English resource from your own 4.0.0 `message.sarc`:

```bash
python tools/build_message_sarc.py \
  --input /path/to/original/message.sarc \
  --text config/popup.txt \
  --output romfs/UI/USen/message.sarc
```

See [CUSTOMIZATION.md](CUSTOMIZATION.md) for text customization.

## Local configuration

Repository defaults live in `config.mk`. Put machine-specific overrides in an ignored `config.local.mk` instead of committing them.

Start from:

```bash
cp config.local.mk.example config.local.mk
```

Then edit values such as `FTP_IP` and `RYU_PATH`.

## Optional FTP deployment

Create a virtual environment and install the deployment dependency:

```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
```

Then deploy the built `exefs` files:

```bash
make deploy-ftp
```

`make deploy-ftp` uploads files from `deploy/` only. It does **not** upload `romfs/UI/USen/message.sarc`; copy that file separately when the popup resource changes.

## Code map

| Path | Purpose |
| --- | --- |
| `source/beerio/custom_selector.cpp` | Custom Frequency/Start selectors and value formatting |
| `include/beerio/text_config.hpp` | User-facing names of the two custom VS settings |
| `source/beerio/reminder_settings.cpp` | Race scheduling state and reminder rule |
| `source/beerio/reminder_dialog.cpp` | Native Yes/No dialog request and result handling |
| `source/beerio/hooks.cpp` | VS menu injection, race-start gate, and dialog completion hooks |
| `include/beerio/addresses.hpp` | MK8DX 4.0.0 module offsets |
| `config/popup.txt` | Default popup text |
| `tools/build_message_sarc.py` | Adds/updates popup label 60000 in `Menu.msbt` |

## Version support

All hard-coded offsets are for MK8DX **4.0.0**. Do not claim support for another version until the offsets and affected structures have been re-verified.

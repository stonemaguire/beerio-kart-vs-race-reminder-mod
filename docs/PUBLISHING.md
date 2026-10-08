# Publishing

## GitHub

Before publishing a release:

1. Confirm the game version in `README.md` and `docs/reversing/versions.md`.
2. Build from a clean tree with `make clean && make`.
3. Generate `message.sarc` from your own 4.0.0 US English archive using `tools/build_message_sarc.py`.
4. Test Frequency/Start scheduling, **No**, and **Yes** in-game.
5. Keep generated binaries, extracted game files, logs, virtual environments, and machine-specific configuration out of the repository.
6. Tag the release (for example `v1.0.0`) and attach the install ZIP to the GitHub release.

The GitHub source tree should not contain an extracted Nintendo `message.sarc`; only the release/install package needs the generated LayeredFS resource.

## Release ZIP layout

```text
atmosphere/
└── contents/
    └── 0100152000022000/
        ├── exefs/
        │   ├── main.npdm
        │   └── subsdk9
        └── romfs/
            └── UI/USen/message.sarc
```

Users should be able to copy the `atmosphere` folder directly to the root of the SD card.

## GameBanana

Ready-to-paste listing copy is in [GAMEBANANA.md](GAMEBANANA.md).

Recommended screenshots:

- Local VS settings showing the two added options.
- The reminder popup before a race.

State compatibility prominently: **MK8DX 4.0.0, Atmosphere, Local VS Race, US English popup resource**.

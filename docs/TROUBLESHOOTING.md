# Troubleshooting

This page collects step-by-step fixes for the launcher and game problems that keep
coming up in GitHub issues. It is **community troubleshooting, not official
documentation**: it was not written by the maintainers, and it is not a substitute
for the project's own wiki. Where a fix is a workaround rather than a real solution,
this page says so.

Every entry has the same shape:

- **What you see** — the symptom, in the words you would use to search for it.
- **Why** — one line on the cause.
- **Steps** — numbered, with the exact command or the exact click.

If a workaround only applies to a specific Minecraft version, that is stated
explicitly. Nothing here is a patch to the launcher; see the linked issues for the
real fixes.

## First: get the game log

Most problems are diagnosable from the log, so get it first.

1. Open the launcher UI.
2. Click the **gear icon** in the top-right corner.
3. Tick **Show log when starting the game**.
4. Start the game. The log window shows messages in real time, and it also opens
   automatically if the game crashes. Use the icon in the top-right of the log
   window to copy the whole thing.

The log window also has a **Run troubleshooter** button in the same settings menu,
which checks a few common misconfigurations for you.

## The game crashes at startup with `dlopen failed: cannot locate symbol "pthread_sigmask"`

**What you see:** the game never reaches the main menu. The log ends with
`cannot locate symbol "pthread_sigmask" referenced by libminecraftpe.so` and the
process dies with `Signal 11` (segmentation fault).

**Why:** Minecraft 1.26.x calls `pthread_sigmask`, a C-library function that exists
on Android (Bionic libc). The `libc-shim` used by your launcher build does not export
that symbol yet, so the dynamic linker refuses to load `libminecraftpe.so`.

**Status:** tracked in [#1970](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1970).
It is **not fixed** in the launcher source at the time of writing.

**Steps:**

1. Update the launcher to the newest build you can get (see
   [Updating the launcher](https://minecraft-linux.github.io/troubleshooting/index.html#updating-the-launcher))
   and try again.
2. If it still crashes, use a Minecraft version older than 1.26.33.1: open the
   launcher's **Versions** tab, install a 1.26.33.x-or-older build, and launch that.
   This is a workaround, not a fix.
3. Do **not** install `libfmod` from your distro's package manager to try to fix
   this. The launcher does not load the system copy of libfmod, and a maintainer
   confirmed in #1970 that it has no effect on this crash.

## The launcher exits with code 6, or the log mentions a Mesa / DRI / EGL error

**What you see:** launching fails, or you get a black window. The log mentions a Mesa
DRI or EGL problem, and people often report it as "exit code 6".

**Why:** your 32-bit or 64-bit Mesa EGL/GLX driver is missing or broken, so the
launcher cannot create an OpenGL context. The number "6" is not a launcher-defined
error code, so treat it as a symptom, not as a diagnosis.

**Status:** tracked in [#1974](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1974) (open).

**Steps:**

1. Find out which renderer you actually got. First list your installed versions:
   `ls ~/.local/share/mcpelauncher/versions/`
   then run the game from a terminal and look for `llvmpipe` or `swrast`:
   `LIBGL_DEBUG=verbose mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/<version> 2>&1 | grep -i "direct rendering\|swrast\|llvmpipe"`
   `llvmpipe` or `swrast` means you are on software rendering — that is the thing to fix.
2. Install the Mesa drivers. On Debian / Ubuntu / Linux Mint:
   `sudo apt update && sudo apt install -y libgl1-mesa-dri libglx-mesa0 libegl1 libvulkan1`
3. If you have an NVIDIA card, also install the matching GL driver. Find your driver
   version with `nvidia-smi`, then install the package for it, for example:
   `sudo apt install libnvidia-gl-535`
4. Retry the game.
5. If a game still refuses to start on Mesa, force a desktop GL version for that one
   run: `MESA_GL_VERSION_OVERRIDE=4.5COMPAT mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/<version>`

If the log instead shows `EGLUT: failed to initialize EGL display`, the same driver
installation applies — see the upstream entry linked at the bottom.

## The game freezes after ~30 seconds, or freezes when you click anything in the menu

**What you see:** the game starts and runs, then locks up — roughly 30 seconds in, or
as soon as you click a button in the main menu or try to join a world or a Realm.
Sometimes it exits silently instead.

**Why (best current understanding):** the launcher injects hooks into
`libminecraftpe.so` at fixed offsets, and those offsets are aligned to one specific
Minecraft build. If the binaries you are running are newer than the launcher's hook
offsets — or you copied a newer build into an older version folder — the hooks point
at the wrong addresses and the game hangs or segfaults. This is a version-mismatch
problem, not a GPU-driver problem.

**Status:** tracked in
[#1973](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1973) and
[#1972](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1972) (both open).

**Steps:**

1. Do not mix versions. If you manually renamed or moved a newer Minecraft build into
   an older version folder, delete it — that is the most common trigger.
2. In the launcher UI, open the **Versions** tab and delete the affected version.
3. Re-download the version the launcher itself offers, and launch it unchanged.
4. Update the launcher to the newest build. Hook support for new Minecraft releases is
   added over time, so a newer launcher is often the only real fix.
5. If the freeze started only after you enabled an option in `options.txt`, remove that
   change and retest.

> Older drafts of this guide suggested editing a `gfx_multithreaded_renderer` value in
> `options.txt`. That option is not present in the launcher source, so it is not a
> reliable fix and is not recommended here.

## "The Launcher failed to sign you in" with `Error=MissingDroidguard`

**What you see:** after you enter your Google credentials, the launcher says *The
Launcher failed to sign you in* or *Please login again*, and the response contains
`No auth cookie field returned ... Error=MissingDroidguard`.

**Why:** Google's sign-in flow asks for a device-attestation step (Play Integrity /
Droidguard) that cannot run outside a real Android device, so the auth cookie is
withheld.

**Status:** tracked in [#1631](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1631) (open).

**Steps:**

1. Open <https://myactivity.google.com/activitycontrols> in a browser, signed in to the
   **same Google account** you use in the launcher.
2. Find **Web & App Activity** and toggle it — turn it off if it is on, or on if it is
   off. The point is to *change* the setting, not a particular direction.
3. Retry the login in the launcher.
4. If it still fails, retry a few times. This check is flaky, and several people in
   #1631 got in after a few attempts.
5. Update the launcher to the newest build and try again — the sign-in handling was
   patched after this was reported.

This is the same workaround as the upstream wiki's
["No auth cookie field returned"](https://minecraft-linux.github.io/troubleshooting/index.html#no-auth-cookie-field-returned)
entry. There is no permanent fix inside the launcher.

## No sound, `unable to open slave`, or sound stops when you alt-tab

**What you see:** no audio at all, or audio cuts out the moment the game window loses
focus.

**Why:** the OpenAL / PulseAudio / PipeWire backend is missing or drops its stream.

**Steps:**

1. Install the audio stack. Debian / Ubuntu: `sudo apt install libasound2 libpulse0`
   (add the `:i386` variants if you run a 32-bit game). Arch: `lib32-libpulse lib32-alsa-plugins`.
   PipeWire users: install your distro's `pipewire-alsa` package.
2. If several backends are installed, tell OpenAL which order to try, then start the
   game from that same terminal:
   `export ALSOFT_DRIVERS=pulse,alsa,jack`
3. Note that in-game music is a separate Marketplace download and is not present by
   default, so "no music" alone is not a bug.

## The launcher will not download the version you want

**What you see:** the version does not appear in the list, or the download fails with a
message about an unverified or unsupported version.

**Why:** the launcher only lists versions it has verified. Newer and beta releases can
be missing until they are checked.

**Steps:**

1. Open the launcher UI and open **Settings**.
2. Scroll to the **Dev** section and enable **Show unverified versions**.
3. Reopen the **Versions** tab — the version should now be listed.
4. Understand that this is unsupported: the launcher itself warns *"Please don't report
   this error. Disable show unverified Versions and reopen the Game..."*.
5. If you already have the APK, import it with `mcpelauncher-extract` rather than
   renaming files into an existing version folder.

## See also

Official and upstream resources (all links checked 2026-10-05):

- Upstream troubleshooting wiki — the project's own page, prefer it over this file:
  <https://minecraft-linux.github.io/troubleshooting/index.html>
- Software rendering / `EGLUT: failed to initialize EGL display`:
  <https://minecraft-linux.github.io/troubleshooting/index.html#graphics-performance-issues-software-rendering---eglut-failed-to-initialize-egl-display>
- `No auth cookie field returned` (MissingDroidguard):
  <https://minecraft-linux.github.io/troubleshooting/index.html#no-auth-cookie-field-returned>
- Forcing software rendering (referenced by the README), issue #933:
  <https://github.com/minecraft-linux/mcpelauncher-manifest/issues/933>
- Launcher UI project:
  <https://github.com/minecraft-linux/mcpelauncher-ui-manifest>
- Nightly Debian / Ubuntu / Fedora packages:
  <https://github.com/minecraft-linux/pkg>

Related GitHub issues:

- [#1970](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1970) —
  `pthread_sigmask` crash on Minecraft 1.26.x
- [#1974](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1974) —
  exit code 6 / Mesa DRI
- [#1973](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1973) —
  game freezing after ~30 seconds
- [#1972](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1972) —
  freeze on menu interaction
- [#1631](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1631) —
  `Error=MissingDroidguard` login failure

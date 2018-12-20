linux-steam-integration
-----------------------

Linux Steam Integration is a software package aimed squarely at Linux distribution developers.
It is designed to offer a far better Steam* experience than the stock client, doing away with old
integration issues and crashes.

### Native Libraries

LSI enforces the use of host-native libraries, both in the Steam Client (via `lsi-intercept`) and
games themselves. This ensures Steam integrates properly, games run with the latest libraries
and users get the best performance.

Currently the officially provided Steam runtime is based on very old Ubuntu 12.04 packages,
and has some cherry-picked updates (SDL, for example). This causes lots of integration issues
on most newer distributions, such as broken client behaviour with full screen videos, outright
crashes on startup, mismatched C++ ABI, missing sound, ugly menus, broken font rendering,
poor performance, etc.

In a nut shell, LSI will force most of Steam over to using native libraries. The intercept
system is new as of 0.4, and will be extended in future to allow improving certain games
on a process-name whitelisting basis.

### Configurable

You can easily toggle the use of the native runtime or the runtime provided with the official
Steam package distribution. LSI doesn't butcher any files, so your local Steam directory does
not need to be repaired or reinstalled. Just run `lsi-settings` and restart Steam.

Currently you can control whether the native runtime is used, force 32-bit mode for some troublesome
ports, and control whether the intercept library is enabled (which will undo the various library
mangling tricks of the Steam distribution to force the usage of the system libs via rtld-audit)

### Portable

You can (and should) stick this into any distro. Hit us up if you need help integrating, or need some
portability changes made. We won't bite.

### Going beyond LSI

LSI solves a great many issues, however there is then still a responsibility for the integrating distribution
to provide basic ABI compatibility, as well as being functionally comparable to the "stock" runtime.
Our future plans will basically involve creating a project based around LSI and a specially produced,
optimised Steam runtime, usable on all Linux distributions.

If you want to be part of that effort - and help bring a modern, optimised gaming experience to your Linux
users, please get in contact. Let's do this once, and do it right.


## Integrating LSI

To correctly integrate LSI, your Steam package will require modification. LSI must provide the `/usr/bin/steam binary`, so your Steam package must move the main launcher to a shadow location.

For users who do not have access to LSI in their distribution, because it has not been integrated yet, please configure with `-Dwith-shim=co-exist` to preserve your existing Steam integrity. Then use the "LSI Steam" shortcut in your menu to launch Steam via LSI.

**Configuring LSI build**

There are a number of meson configure options you should be aware of when integrating LSI correctly. The prominent ones are explained here.

`-Dwith-shim= none | co-exist | replacement`

        Control behaviour of the shim build. `none` will disable building the main
        shim, required for actually launching Steam in the first place.
        
        `co-exist` will ensure this does not conflict with the `/usr/bin/steam`
        path, providing a new `lsi-steam` binary and desktop file.

        `replacement` will provide a drop-in replacement `/usr/bin/steam` binary
        shim responsible for bootstrapping Steam via LSI. This will require you
        to mask the original steam binary to a new location.

        The default value for this option is:

                `replacement`


`-Dwith-steam-binary=$PATH`

        Set the absolute path for the shadowed Steam binary.
        LSI will execv Steam after it has initialised the environment.
        Note that execv, not execvp, is used, to mask programs that may be
        in the `$PATH`, hence requiring an absolute path.

        The default value for this option is:

                `/usr/lib/steam/steam`

        This option is only applicable for distribution integrators, when
        Steam replacement is enabled.


`-Dwith-preload-libs=$LIBS`

        A colon separated list of libraries that are required to launch Steam
        when using its own runtime. LSI enables users to switch back to the Steam
        runtime, and in this instance we manage the LD_PRELOAD environmental variable.
        Ensure this is correct, and escape $LIB for correct Linux usage.

        The default value for this option is:

                `/usr/\$LIB/libX11.so.6:/usr/\$LIB/libstdc++.so.6`

`-Dwith-new-libcxx-abi=$boolean`

        Enable this if your distribution uses a recent libstdc++ build, from
        GCC 6 or higher.

`-Dwith-frontend=$boolean`

        A small UI application is shipped to enable configuration of LSI, which presents
        a simple GTK3 Dialog to the user. It is not enabled by default, it is up
        to the integrator to decide if they wish to employ lsi-settings, or implement
        an alternative.

        The lsi-settings application will only ever write new configurations to the
        **user** settings file, and requires no special permissions.

        The default value for this option is:

                `false`

`-Dwith-libressl-mode= none | native | shim`

        Control how LSI's `liblsi-intercept.so` handles LibreSSL.

        The intercept module can be configured to enhance security of Linux ports
        by ensuring they use up to date LibreSSL builds.

        When set to `native`, it is assumed the distro is employing LibreSSL as the
        default libSSL ABI (i.e. `libssl.so.1.0.0`)

        When set to `shim`, LSI will redirect name requests for the `libssl` and
        `libcrypto` libraries to a uniquely named LibreSSL build of libraries,
        allowing libressl + openssl to co-exist on disk.

        The default value for this option is:

                `none`

`-Dwith-libressl-suffix=$suffix`

        This option is only used when `-Dwith-libressl-mode=shim`, and sets the
        suffix for the libraries that LSI will attempt to use when transforming
        requests for LibreSSL libraries.

        This will transform `libssl.so.$VERSION` to `libssl-$suffix.so`, i.e.
        `libssl.so.37` would become `libssl-libressl.so`. This allows distros
        to provide up to date `libressl` and `openssl` builds in parallel to
        improve the security of games relying on LibreSSL.

        The default value for this option is:

                `-libressl`

## How LSI Works

LSI provides a /usr/bin/steam binary to be used in place of the existing Steam script, which will then correctly set up the environment before swapping the process for the Steam process.
When using a full build of LSI, the bootstrap shim will force a correct environment, removing any issues that can cause various older SDL versions to kill Steam. Additionally it will then
force `STEAM_RUNTIME=0` if this is set in the config (default behaviour).

When using the native runtime, and the `intercept` library is enabled, the LSI shim will also set up `LD_AUDIT` to use `liblsi-intercept.so`. This library will intercept all dynamic linking operations
for the main Steam binaries and override their behaviour, allowing Steam to only load a handful of vendored libraries (`libav` fork) as well as its own private libraries. It is then forced to use
system libraries for everything else. This means the Steam client, web helper, etc, will use the native SDL, X11, fixing many bugs with video playback, font rendering and such.

As of the `0.6` release of LSI, we now also support a `redirect` module. When enabled, the LSI shim will set `LD_PRELOAD` to use `liblsi-redirect.so`, which will conditionally enable it's own
internal logic based on the game being played. This module currently overrides the `open()` and `fopen64()` system calls to apply dynamic bug fixes to games, and may be expanded in future to
provide more fixes.

Currently the `redirect` module supports:

 - ARK: Survival Evolved* (Use fixed shader asset from `TheCenter` DLC to fix invalid water appearance)
 - Project Highrise* (Fix crash where the game attempts to use a directory as a config file `prefs.txt`)

If you are packaging LSI for a distribution, please ensure you provide both a 32-bit and 64-bit build of the intercept library so that the entirety of Steam's  library (`.so`) mechanism is tightly
controlled by LSI. See the scripts in the root directory of this repository for examples of how to do this.

Configuration options can be placed in an INI-style configuration file in a series of locations, which are ordered by priority in a cascade:

        ~/.config/linux-steam-integration.conf
        /etc/linux-steam-integration.conf
        /usr/share/defaults/linux-steam-integration/linux-steam-integration.conf

The user configuration takes immediate priority. Secondly we have the system-wide configuration for affecting all accounts, and lastly the vendor configuration, which may be provided by the integrator.

Currently this INI file supports three options. The root section in this INI file must be `[Steam]`.

`use-native-runtime = $boolean`

        If set to a true boolean value, (yes/true/on), the host OS's native runtime
        will be used instead of the Steam provided runtime. If this is set to
        a false boolean value (no/false/off), then the startup will be modified
        to export the relevant `LD_PRELOAD` and `STEAM_RUNTIME` settings.

        The default value of this variable is `true`.

`force-32bit = $boolean`

        If set to a true boolean value (yes/true/on), the shadowed Steam binary will
        be run via the `linux32` command. This will force the `steam` process
        and all children to believe they are running on a 32-bit system. This
        may be useful for 64-bit games that are buggy only on 64-bit.

        If this is set to a false boolean value (no/false/off), then the
        shadowed Steam binary will be directly executed. Note that on 32-bit
        OSs this setting is ignored.

        The default value of this variable is `false`.

`use-libintercept = $boolean`

        If set to a true boolean value (yes/true/on), the `liblsi-intercept.so`
        library will be utilised to control dynamic linking within the Steam client
        and associated processes.

        Note this requires `use-native-runtime` to be set.

        The default value of this variable is `true`.

`use-libredirect = $boolean`

        If set to a true boolean value (yes/true/on), the `liblsi-redirect.so`
        library will be utilised to apply "hot" workarounds to certain games to
        fix various runtime issues.

        Note this requires `use-native-runtime` to be set.

        The default value of this variable is `true`.

`use-unity-hack = $boolean`

        If set to a true boolean value (yes/true/on), the `liblsi-redirect.so`
        library will apply a preference workaround to Unity3D based games and
        abstract access to their configuration files. This is used to force
        Unity3D games to always start in windowed mode, and to ensure that
        they're unable to make use of the stored fullscreen setting.


## Common issues

Virtually all issues will come down to having the right dependencies. LSI puts an emphasis on
host-first libraries, so make sure all the correct libraries are available for both 32-bit and
64-bit.

Steam has both 32-bit and 64-bit processes/libraries, having the full dependency set for both
the client and common games/engines is a must. 

### Steam fails to launch with libintercept/native-runtime

Make sure you enable full debugging when first assigning dependencies to your runtime package:

```bash
$ LSI_DEBUG=1 steam
# If you use co-exist option:
$ LSI_DEBUG=1 lsi-steam
```

### liblsi-intercept regressing performance

There exists a bug in `glibc` which incorrectly configures profiling for all PLT calls when using `LD_AUDIT` (rtld-audit)
even if the audit library doesn't implement `la_pltenter` or `la_pltexit`. This leads to a performance hit on every
runtime call via PLT.

The issue was first reported to this project by @amonakov [here](https://github.com/solus-project/linux-steam-integration/issues/15).
The upstream glibc issue is reported on the upstream glibc [bugzilla](https://sourceware.org/bugzilla/show_bug.cgi?id=15533).
A patch to resolve the issue was submitted by @amonakov [here](https://sourceware.org/ml/libc-alpha/2013-05/msg00888.html).

If you are noticing performance regressions, you can:

 - As a user: Disable `liblsi-intercept` via `lsi-settings`. This will hurt compatibility.
 - As a packager: Disable `liblsi-intercept` via build flags. This will hurt compatibility.
 - As a distribution integrator: Add weight to upstream issue and import patch into your distribution.
   This will retain LSI compatibility magic and mitigate performance issues.
   As an example, Solus has already [integrated the patch](https://dev.getsol.us/R927:afa5b639e8a9b62618457a304d1e6fb42a9f2066).


`* Some names may be claimed as the property of others.`

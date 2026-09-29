# Nix and NixOS

Scratch notes from the shipping-code lecture and chat.
General reference for this repo.

## What Nix and NixOS are

Nix: package and build system with a content-addressed store (`/nix/store/...`).

NixOS: Linux distribution; system config (services, packages, users) in Nix language.

Nix runs on Ubuntu or macOS while you keep that host distro.

## Flakes

Shipping lecture: manage system libraries and tools per project via flakes.

Repo-root `flake.nix` declares inputs (nixpkgs commit, other flakes) and outputs (dev shells, packages, NixOS modules).

`flake.lock` pins input revisions like `uv.lock` for nixpkgs.

`nix develop` enters a shell with toolchain and native libs from that lock.

`nix flake update` refreshes inputs and the lock file.

Per-repo flakes pin host-side build deps apart from global apt.

Beside Docker: venv locks Python; images ship runtimes; flakes lock the dev and build host slice.

Hermetic builds: pin compiler, system libs, and build env from the same files.

## Declarative vs apt

Desired state lives in files (`flake.nix`, `configuration.nix`).

`nixos-rebuild switch` or `nix develop` realizes that state; old generations support rollback.

apt on Ubuntu: imperative `apt install` mutates the system; reproducing another host needs scripts or memory.

## Compared to Ubuntu

nixpkgs covers many of the same upstream apps with different attribute names and pinned nixpkgs commits.

Versions follow your flake or channel pin.

Layout uses store paths and profiles rather than one shared FHS `/usr` lib tree.

Multiple package versions can coexist in the store.

## NixOS lineage

NixOS is an independent distribution built on nixpkgs.

Ubuntu derives from Debian (`.deb`, `apt`).

Both run Linux; packaging and filesystem layout differ.

A `.deb` targets Debian-shaped paths and `Depends` on Debian package names.

## Developer packaging for Nix

Upstream wheels and crates work as usual; Nix adds a separate derivation or flake.

Someone adds a Nix derivation in nixpkgs or ships a `flake.nix` in the repo.

Reproducible tags, lockfiles, and documented build steps help packagers.

## User: same niche software as Ubuntu?

Popular tools are usually in nixpkgs.

Long tail tools may need a custom derivation, Flatpak, or Ubuntu userland in distrobox.

Vendor `.deb` only: distrobox or VM plus `apt` matches Ubuntu install closely.

## AppImage via Nix recipe

Common nixpkgs pattern: `fetchurl` with fixed hash, install AppImage, wrapper script, desktop entry.

Optional extras: FUSE, nixGL for GPU, `patchelf`.

Manual run: download AppImage, `chmod +x`, run (with forum fixes when needed).

## `.deb` via Nix recipe

Same idea possible: fetch deb, unpack, copy to `$out`, `autoPatchelfHook`, wrappers.

`.deb` often declares Depends on system libs under `/usr`; more FHS emulation or patchelf than many AppImages.

`buildFHSUserEnv` and nix-ld help Ubuntu-linked binaries.

## distrobox and VM

distrobox: container with Ubuntu (or Fedora) userland; `apt install` inside; integrated CLI/GUI.

VM: full Ubuntu when you want an entire Debian-shaped OS.

Flatpak is another channel on NixOS when upstream ships it.

## systemd and containers on NixOS

NixOS still runs systemd; `systemctl` works at runtime.

Machine config lives in Nix (`configuration.nix` / flake), then `nixos-rebuild switch` materializes `/etc` and units.

`virtualisation.docker.enable` starts Docker; a `systemd.services.*` block can run `docker compose up -d` like the Ubuntu lecture unit.

`virtualisation.oci-containers` declares containers directly in Nix as an alternative to Compose YAML.

## Ubuntu vs NixOS for “clone my machine”

NixOS: one declarative config, generations, rollback via rebuild.

Ubuntu: reproducible setups use Ansible, cloud-init, or golden images in git; same goal, more layers than a single switch.

Nix on Ubuntu adds flake-pinned dev envs while apt still owns much of the system tree.

## Lecture tie-in

NixOS: version-controlled config for a whole machine clone.

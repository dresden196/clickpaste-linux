# AUR Package

This directory contains the PKGBUILD for submitting ClickPaste to the Arch User Repository (AUR).

## Files

- `PKGBUILD` - Standard release package (uses tagged releases)
- `PKGBUILD-git` - Git development package (builds from latest commit)
- `.SRCINFO` - Package metadata (regenerate after PKGBUILD changes)

## Before Submitting to AUR

1. **Update URLs** in PKGBUILD and .SRCINFO:
   - Replace `YourUsername` with your actual GitHub username
   - Update maintainer email

2. **Create a GitHub release** with a tag (e.g., `v1.0.0`)

3. **Update sha256sums**:
   ```bash
   # Download the release tarball and generate checksum
   curl -L https://github.com/YourUsername/clickpaste-linux/archive/v1.0.0.tar.gz | sha256sum
   ```

4. **Regenerate .SRCINFO**:
   ```bash
   makepkg --printsrcinfo > .SRCINFO
   ```

## Submitting to AUR

1. **Create an AUR account** at https://aur.archlinux.org/

2. **Add your SSH key** to your AUR account

3. **Clone the AUR package** (first time):
   ```bash
   git clone ssh://aur@aur.archlinux.org/clickpaste.git
   cd clickpaste
   ```

4. **Copy files**:
   ```bash
   cp /path/to/PKGBUILD .
   cp /path/to/.SRCINFO .
   ```

5. **Commit and push**:
   ```bash
   git add PKGBUILD .SRCINFO
   git commit -m "Initial upload: clickpaste 1.0.0"
   git push
   ```

## Testing Locally

```bash
# Test the PKGBUILD
makepkg -si

# Check for issues
namcap PKGBUILD
namcap clickpaste-1.0.0-1-x86_64.pkg.tar.zst
```

## For the -git Package

The `-git` package is for users who want the latest development version. Submit it as a separate AUR package named `clickpaste-git`.

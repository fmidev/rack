# Releasing Rack

## One-time setup

- Create a Quay robot account with write access to `fmi/rack`. Add its credentials
  as GitHub repository Actions secrets named `QUAY_USERNAME` and `QUAY_PASSWORD`.
- Disable the existing Quay.io build trigger for `fmi/rack` before releasing.
  Otherwise Quay's untested build could overwrite the images tested by GitHub Actions.

## Publish a release

1. Bump `RACK_VERSION` in `src/main/rack.h` to the desired version and commit it.
2. Push the commit to `main` and wait for CI to pass. Python test failures are
   temporarily allowed.
3. Tag that commit and push the tag (replace `X.Y.Z` with the source version):

   ```sh
   git tag vX.Y.Z && git push origin vX.Y.Z
   ```

The release workflow verifies that the tag matches `RACK_VERSION`, builds the
Ubuntu 22.04 container, and smoke-tests its version and help commands. Only after
those checks pass does it publish `quay.io/fmi/rack:vX.Y.Z` and
`quay.io/fmi/rack:latest`. It then creates a GitHub Release with generated notes
and GitHub's automatic source archives.

## Planned follow-ups (not part of this setup)

- Submit and maintain a conda-forge recipe.
- Choose the optimal container base images.
- Add Python to the container image.
- Make the Python tests pass in CI and remove their allowed-failure status.

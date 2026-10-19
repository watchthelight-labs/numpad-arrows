# Contributing

The most useful report is whether it works on your keyboard. Open an issue with the laptop or keyboard model, the macOS build, whether `hidutil` remapping worked for it, and what `launchctl print gui/$(id -u)/org.watchthelight.numpad-arrows` says about state and last exit code. If a key does nothing or the wrong thing, say which key and which app.

Pull requests: keep them small and say how you tested them. I read and test every PR before merging. AI-assisted PRs are fine as long as you can explain the change, which is the same bar I hold my own code to. A different mapping for your own keyboard belongs in your copy of `kMap`, not in a PR, unless it is behind an option that keeps the default as it is.

Contributions are under the ISC licence, like the rest of the repo.

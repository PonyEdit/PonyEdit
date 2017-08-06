PonyEdit
========

Coding at the speed of thought.

## About PonyEdit

PonyEdit is a remote SSH text editor like none you've ever seen before. Editing your files feels as fast and slick as editing local files, and your changes are saved nearly instantly to the remote server at the press of a button.

## How it Works

PonyEdit opens your files in a fully local text editor; allowing you to make changes as quickly as if you were editing a local file. However, it keeps an SSH connection open in the background and streams your changes to the remote site as you edit, so when it the time comes to save, your changes are already on the server and saving is nearly instant!

## Server Requirements

PonyEdit operates over a standard SSH shell connection, and only requires Perl on the remote server. There are no other special requirements, and it doesn't open any additional ports.

## Build Requirements

PonyEdit compiles with the latest version of [Qt](https://www.qt.io/).

On OSX, you'll need:

* XCode, available from the [Mac App Store](https://itunes.apple.com/au/app/xcode/id497799835?mt=12). Remember to open it and accept the Terms and Conditions, you need to do that before the command line tools will work.

On Windows, you'll need:

* [LLVM/Clang](http://releases.llvm.org/download.html)

On Linux, you'll also need to install these packages:

* `libssl-dev`
* `libssh2-1-dev`

## Getting PonyEdit

You can download the latest version of PonyEdit [here](https://github.com/PonyEdit/PonyEdit/releases).

## License

PonyEdit is released under the GPL Version 3. This software is released as-is, use at your own risk, etc.

## Authors and Contributors

PonyEdit was originally created by [@thingalon](https://github.com/thingalon) and [@pento](https://github.com/pento).

If you like PonyEdit and would like to contribute, please submit a pull request; we'd love your help! We are particularly interested in volunteers to maintain Windows and Linux binary releases for it.

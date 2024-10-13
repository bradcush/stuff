# stuff

Command-line dotfiles management

## About

stuff is a tool intended to make it simple to manage dotfiles using a
repository with version control. Like some alternatives - stow or chezmoi -
it's based on linking but those tools are incomplete for some basic use cases
that I was looking for.

## C language

This project is meant to be written mostly in C, a good choice for systems
programming over many decades. While other, newer languages are suitable like
Zig or Rust, I've chosen C for the simple fact that it's fast and I just want
to experience writing a robust command-line tool with it.

## Building

Building from source only:

`make` or `make stuff`

## Running

See `./stuff --help` for commands and options

## Documentation

I've tried to make things as intuitive as I can with sane defaults. All
documentation related to usage is included in the tool itself which can be
accessed through the `--help` flag corresponding to optional commands.

## Architecture

Parsing options and commands is done in a hierarchical, two-step manner where
global (hidden) options are parsed first and then command options. This gives
flexibility but also requires care in how the two may affect one another. Keep
in mind that top-level hidden options are meant for internal use only.

## Quirks

Option parsing using `getopt` has quirks when used in a hierarchical manner
which we code around for now. Some global options may interact with some
command options and vice versa.

If command options take values then global options need to be aware to avoid
unwanted behaviour. If we specify a global option then all command options need
to also be aware to avoid throwing. These cases are expected to be few in
number so we live with some sharp edges for the time being to support them
until we find a better way.

We also parse commands using `optind` which means they don't have to directly
follow the program name when specified. We consider this behaviour a feature
and not a bug for the time being.

## Support

The goal is that stuff will be available on most systems but for now I've only
developed and tested locally using Linux and specifically Arch Linux. It may or
may not work on other distributions of Linux for now.

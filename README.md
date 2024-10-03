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

## Support

The goal is that stuff will be available on most systems but for now I've only
developed and tested locally using Linux and specifically Arch Linux. It may or
may not work on other distributions of Linux for now.

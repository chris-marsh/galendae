# Galendae

A basic popup calendar that can be styled to match workspace themes.

**Galendae** was designed to be a stylish popup calendar that can match the styling of Desktop Environments or Window Managers.

[![sample screenshot](http://i.imgur.com/G4HiuPzb.png)](http://i.imgur.com/G4HiuPz.png)
[![sample screenshot](http://i.imgur.com/84ScLynb.png)](http://i.imgur.com/84ScLyn.png)
[![sample screenshot](http://i.imgur.com/5fM1v8db.png)](http://i.imgur.com/5fM1v8d.png)

## Whats in a name

Galendae is derived from the Roman word [Kalendae](https://en.wikipedia.org/wiki/Calends), meaning the first day of the month. I thought Kalendae sounded like a KDE application and since I was using GTK+, Galendae was born.

## Dependency

    GTK+ 3.x

## Building from source

    $ git clone https://github.com/chris-marsh/galendae.git
    $ make

## Configuration

galendae will look for a configuration file called galendaerc. It will search in the following order;

    ./galendaerc
    ~/.config/galendae/galendaerc

You can specify alternatives configuration files with the '-c' option. You can specify a full path or just a filename. If you only give a filename, the same directories as above will be tried.

## Running

    $ galendae
    $ galendae -c examples/blue-config

## Useage

    galendae [OPTION ...]

    DESCRIPTION

        Galandae displays a gui calendar. Keys:

            h|Left    - decrease month
            l|Right   - increase month
            k|Up      - increase year
            j|Down    - decrease year
            g|Home    - return to current date
            q|Esc     - exit the calendar

    OPTIONS
        -c, --config FILE   - config file to load
        -h, --help          - display this help and exit
        -v, --version       - output version information


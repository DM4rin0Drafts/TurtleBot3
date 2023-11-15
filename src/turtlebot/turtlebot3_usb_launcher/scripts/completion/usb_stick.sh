#!/bin/bash

function usb_stick() {
    command="$1"
    shift

    if [[ "$command" == "help" || "$command" = "--help" || -z "$command" ]]; then
        _usb_stick_help
        return 0
    fi

    case $command in
        format)
            $(rospack find turtlebot3_usb_launcher)/scripts/helper/usb_create_stick.sh "$@"
            ;;

        make)
            $(rospack find turtlebot3_usb_launcher)/scripts/helper/usb_make_and_copy.sh "$@"
            ;;

        restart)
            $(rospack find turtlebot3_robot_bringup)/usb_stick_scripts/usb_stick_restart.sh "$@"
            ;;

        eject)
            $(rospack find turtlebot3_robot_bringup)/usb_stick_scripts/usb_stick_eject.sh "$@"
            ;;

        *)
            echo "Unknown command: $command"
            _usb_stick_help
            ;;
    esac
}

function _usb_stick_commands() {
    local ROSWSS_COMMANDS=('help' 'format' 'make' 'restart' 'eject')
    
    echo ${ROSWSS_COMMANDS[@]}
}

function _usb_stick_help() {
    echo "The following commands are available:"

    commands=$(_usb_stick_commands)

    for i in ${commands[@]}; do
        echo "   $i"
    done
}

function _usb_stick_complete() {
    local cur

    if ! type _get_comp_words_by_ref >/dev/null 2>&1; then
        return 0
    fi

    COMPREPLY=()
    _get_comp_words_by_ref cur

    # roswss <command>
    if [ $COMP_CWORD -eq 2 ]; then
        if [[ "$cur" == -* ]]; then
            COMPREPLY=( $( compgen -W "--help" -- "$cur" ) )
        else
            COMPREPLY=( $( compgen -W "$(_usb_stick_commands)" -- "$cur" ) )
        fi
    fi

    # usb_stick command <subcommand..>
    if [ $COMP_CWORD -ge 3 ]; then
        prev=${COMP_WORDS[2]}

        # default completion
        case $prev in
            format)
                if [ $COMP_CWORD -eq 3 ]; then
                  dev="$(lsblk | grep /media | grep -oP "sd[a-z]" | awk '{print "/dev/"$1}' | sort | uniq)"
                  COMPREPLY=( $( compgen -W "$dev" -- "$cur" ) )
                else
                  COMPREPLY=()
                fi
                ;;

            make)
                _roscomplete
                ;;

            *)
                COMPREPLY=()             
                ;;
        esac
    fi

    return 0
}
complete -F _usb_stick_complete usb_stick

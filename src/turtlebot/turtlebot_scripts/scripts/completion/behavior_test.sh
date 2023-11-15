#!/bin/bash

function roswss_behavior_test() {
    source $ROSWSS_BASE_SCRIPTS/helper/helper.sh

    set -e
	local gui=False
	local postfix='2>/dev/null | grep Testing'
	local help=False
	
	
	
	if [ "$#" == "0" ]; then
		_roswss_behavior_test_help
		return 0
	fi
	local all=False
	local arg
	local launch 
	
	while (( "$#" )); do
		arg=$1
		
		case $arg in
			-g|--gui)
			gui=True
			;;
			-v|--verbose)
			postfix=''
			;;
			-a|--all)
			all=True
			;;
			-h|--help)
			help=True
			;;
			-*)
			help=True
			;;
			*)
			launch=$arg
			break #break the loop
			;;
		esac

		shift

	done

    if [ "$help" = "True" ] || ([ "$launch" = '' ] && [ "$all" = "False" ]); then
        _roswss_behavior_test_help
        return 0
    fi

	local cmd
    if [ "$all" = "True" ] ; then
		local path
    	path="$(rospack find turtlebot3_test_flexbe_behaviors)/tests/"
        for i in `find -L $path -type f -name "*.launch" | sort`; do
		    file=${i#$path}
		    if [ -r $i ]; then
		        cmd="roslaunch turtlebot3_test_flexbe_behaviors $file gui:=\"$gui\" $postfix"
				echo $cmd
				eval $cmd
		    fi
    	done
        return 0
    fi

	while (( "$#" )); do
		launch=$1
		#sleep 5
		cmd="roslaunch turtlebot3_test_flexbe_behaviors $launch gui:=\"$gui\" $postfix"
		echo $cmd
		eval $cmd
		shift
	done
	
    return 0
}

function _roswss_behavior_test_help() {
    echo_note "Type name of behavior to test, double tab to check for available tests. \n\
	--gui\t\tfor visualization.\n\
	--verbose\tfor detailed output.
	--all\t\tto test all available tests."
}

function _roswss_behavior_test_launchfiles() {
    local ROSWSS_LAUNCH_FILES
    ROSWSS_LAUNCH_FILES=()

    local path
    path="$(rospack find turtlebot3_test_flexbe_behaviors)/tests/"
    # find all launch files
    for i in `find -L $path -type f -name "*.launch"`; do
        file=${i#$path}
        if [ -r $i ]; then
            ROSWSS_LAUNCH_FILES+=($file)
        fi
    done
    
    echo ${ROSWSS_LAUNCH_FILES[@]}
}

function _roswss_behavior_test_complete() {
    local cur

    COMPREPLY=()
    _get_comp_words_by_ref cur

    # roswss behavior_test ...
    #if [ $COMP_CWORD -ge 2 ]; then
        if [[ "$cur" == -* ]]; then
            COMPREPLY=( $( compgen -W "--help --verbose --gui" -- "$cur" ) )
        else
            COMPREPLY=( $( compgen -W "$(_roswss_behavior_test_launchfiles)" -- "$cur" ) )
        fi
	#fi
    return 0
}
#complete -W "a b c" roswss_behavior_test
complete -F _roswss_behavior_test_complete roswss_behavior_test

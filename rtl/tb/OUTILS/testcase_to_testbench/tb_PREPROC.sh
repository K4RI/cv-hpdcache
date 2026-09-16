#!/bin/bash

nom_module=$1
sv_dir=$2
input_body=""
output_body=""

############## 1 : distinguer les inputs-outputs grâce au .gates
# au lieu de readarray -t inputs_all < $1.gates ; inputs_all=("${inputs_all[@]:1}")
inputs_all=()
outputs_all=()

while IFS= read -r line; do

    # trim les espaces
    line="${line// /}"
    porte=${line:1}

    # couper avant le premier "[", pour repérer les sig_i[0], sig_o[2]...
    porte_=${porte%%'['*}

    if [[ ${line:0:1} == "i" ]] || [[ ${porte_: -2} == "_i" ]] # ligne "input"
    then
        inputs_all+=($porte)
        input_body="${input_body}"'\n    '"fields[\"${porte}\"]=\&(dut->${porte,,});"
    
    elif [[ ${line:0:1} == "o" ]] || [[ ${porte_: -2} == "_o" ]] # ligne "output"
    then
        outputs_all+=($porte)
        output_body="${output_body}"'\n    '"fields[\"${porte}\"]=\&(dut->${porte,,});"
    fi
done < ${nom_module}.gates


############## 2 : lire les typedef enum dans le .sv et les ajouter au call_input()

typedef_enums=""
inside_def=0
include_024=""
ongoing_vals=""
while IFS= read -r line; do

    # trim les espaces
    line=$(echo "$line" | sed 's/^\ *//')
    if [[ ${line:0:12} == "typedef enum" ]]
    then
        inside_def=1
    
    elif [ $inside_def == 1 ] && [[ $line != "" ]] && [ ${line:0:1} == "}" ]
    then
        nom_type=$(echo $line | cut -d " " -f 2)
        ongoing_vals=(${ongoing_vals//!/ })
        for val in ${ongoing_vals[@]}
        do
            typedef_enums="${typedef_enums}} else if (offre == \"${val^^}\") {"'\n        '"*(V${nom_module}___024unit::${nom_type}*)(fields[porte]) = V${nom_module}___024unit::${nom_type}::${val};"'\n    '
            include_024="#include \"V${nom_module}___024unit.h\""
        done
        inside_def=0
    
    elif [ $inside_def == 1 ] && [[ $line != "" ]] && [ ${line:0:1} != "/" ]
    then
        val=$(echo $line | cut -d " " -f 1 | cut -d "," -f 1)
        ongoing_vals="${ongoing_vals}${val} "
    fi
done < ${sv_dir}${nom_module}.sv


# écrire les actions et nommer le module
sed "s/%INPUT_BODY%/${input_body}/" tb_TEMPLATE.cpp |
sed "s/%OUTPUT_BODY%/${output_body}/" |
sed "s/%TYPEDEF_ENUMS%/${typedef_enums}/" |
sed "s|%IO%|${nom_module}.io|" |
sed "s|%INCLUDE_024UNIT%|${include_024}|" |
sed "s|%NOM_MODULE%|${nom_module}|g" > tb_explore_${nom_module}.cpp

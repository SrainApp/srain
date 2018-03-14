#!/bin/bash

# Type: GtkWidget
# Variations:
#   - GtkWidget
#   - GTK_TYPE_WIDGET
#   - GTK_WIDGET[xxx]
#   - gtk_widget_xxx(

OLD_TYPE=$1
NEW_TYPE=$2

gen_var() {
    cmd=$(cat <<EOF
import re;
l = re.findall('[A-Z][^A-Z]*', '${1}')
print(''.join(l));
print('_'.join(map(lambda x: x.upper(), l[:1] + ['TYPE'] + l[1:])));
print('_'.join(map(lambda x: x.upper(), l)));
print('_'.join(map(lambda x: x.lower(), l + [''])));
EOF
)
    python3 -c "${cmd}"
}

old_var=($(gen_var ${OLD_TYPE}))
new_var=($(gen_var ${NEW_TYPE}))

for f in $(find ./src/ \( -name '*.h' -o -name '*.c' \)); do
    sed -i "s/\b${old_var[0]}\b/${new_var[0]}/g" ${f}
    sed -i "s/\b${old_var[1]}\b/${new_var[1]}/g" ${f}
    sed -i "s/\b${old_var[2]}\([0-9a-zA-Z_]*\b\)/${new_var[2]}\1/g" ${f}
    sed -i "s/\b${old_var[3]}\([0-9a-zA-Z_ ]*(\)\b/${new_var[3]}\1/g" ${f}
done

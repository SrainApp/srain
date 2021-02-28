#!/bin/bash

# Type: GtkWidget
# Variations:
#   - GtkWidget
#   - GTK_TYPE_WIDGET
#   - GTK_WIDGET[xxx]
#   - gtk_widget_xxx(

OLD_TYPE=$1
NEW_TYPE=$2
FILES=${*:3}

gen_var() {
    cmd=$(cat <<EOF
import re;
l = re.findall('[A-Z][^A-Z]*', '${1}')
print(''.join(l));
print('_'.join(map(lambda x: x.upper(), l[:1] + ['TYPE'] + l[1:])));
print('_'.join(map(lambda x: x.upper(), l)));
print('_'.join(map(lambda x: x.lower(), l + [''])));
print('_'.join(map(lambda x: x.upper(), l[:1] + ['IS'] + l[1:])));
EOF
)
    python3 -c "${cmd}"
}

old_var=($(gen_var ${OLD_TYPE}))
new_var=($(gen_var ${NEW_TYPE}))

if [ -z "$FILES" ]; then
    FILES=$(find ./src/ \( -name '*.h' -o -name '*.c' \))
fi

for f in $FILES; do
    sed -i "s/\b${old_var[0]}\b/${new_var[0]}/g" ${f} # GtkWidget
    sed -i "s/\b${old_var[0]}Class\b/${new_var[0]}Class/g" ${f} # GtkWidgetClass
    sed -i "s/\b_${old_var[0]}\b/_${new_var[0]}/g" ${f} # _GtkWidget
    sed -i "s/\b_${old_var[0]}Class\b/_${new_var[0]}Class/g" ${f} # _GtkWidgetClass
    sed -i "s/\b${old_var[1]}\b/${new_var[1]}/g" ${f} # GTK_TYPE_WIDGET
    sed -i "s/\b${old_var[2]}\([0-9a-zA-Z_]*\b\)/${new_var[2]}\1/g" ${f} # GTK_WIDGET[xxx]
    sed -i "s/\b${old_var[3]}\b/${new_var[3]}/g" ${f} # gtk_widget
    sed -i "s/\b${old_var[3]}\([0-9a-zA-Z_ ]*(\)/${new_var[3]}\1/g" ${f} # gtk_widget[xxx](
    sed -i "s/\b${old_var[4]}\b/${new_var[4]}/g" ${f} # GTK_IS_WIDGET
done

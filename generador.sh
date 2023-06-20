#!/bin/bash
# Atención: GNU bc en mi sistema usa punto para separar decimales pero printf usa coma
inicio=$1
cantidad=$2
limite=$(($1+$2))
printf '['
for ((i=$(($inicio)); i<$(($limite)); i++)); do
  value=$(echo "scale=2; (s($i * 2 * 3.14159 / 256 * 4) + 1) * 1.65" | bc -l)
  if [[ $i -ne $(($limite-1)) ]]; then
    printf '"%.2f",' ${value/./,}
  else
    printf '"%.2f"' ${value/./,}
  fi
done
printf ']'

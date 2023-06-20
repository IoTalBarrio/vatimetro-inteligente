#!/bin/bash

printf '['
for ((i=1; i<=1024; i++)); do
  if [[ $i -ne 1024 ]]; then
    printf '%d,' "$i"
  else
    printf '%d' "$i"
  fi
done
printf ']'

#!/bin/bash
# 
# `dot` appication converts graph definition `*.dot` files into
# *.pdf documents containing graphical representation of graphs:
#
for f in *.dot
do
  dot -Tpdf -Nfontsize=12 -Efontsize=8 -Efontcolor=red -Kneato "$f" -o "${f%.dot}.pdf"  
done
read -p "Press ENTER to finish"

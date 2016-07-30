#!/bin/bash
clear
echo "Se va a compilar la Consola"
cd consola
make clean
make all
cd ..
echo ""
echo "==========================================================="
echo "Se va a compilar el CPU"
cd cpu
make clean
make all
cd ..
echo ""
echo "==========================================================="
echo "Se va a compilar el Nucleo"
cd nucleo
make clean
make all
cd ..
echo ""
echo "==========================================================="
echo "Se va a compilar la UMC"
cd umc
make clean
make all 
cd ..
echo ""
echo "==========================================================="
echo "Se va a compilar el SWAP"
cd swap
make clean
make all



LABEL=${LABEL:+"-$LABEL"}
VOLUME=volume${LABEL}.h5

CONV=${CONV:-'30dBZ,5km,2km,20dBZ,0,0'}

#rack $VOLUME --pCappi   1000  -c --palette palette-DBZH.txt  -o conv$LABEL-cappi.png
#rack $VOLUME --pEchoTop 30dBZ -c --palette palette-HGHT.txt  -o conv$LABEL-etop.png

CAPPI="-Q CAPPI  -c --palette palette-DBZH.txt  -o conv$LABEL-cappi.png"
MAX="-Q MAX   -c --palette palette-DBZH.txt  -o conv$LABEL-max.png"
ETOP="-Q ETOP -c --palette palette-HGHT.txt  -o conv$LABEL-etop.png"


cmd="rack $VOLUME --store intermediate=1 --pConv "$CONV" -O conv-polar$LABEL.png -o conv$LABEL.h5 $CAPPI $MAX $ETOP "
echo $cmd 
eval $cmd 

for i in cappi max etop; do
    file=conv$LABEL-$i.png 
    if [ -f $file ]; then
	convert $file -pointsize 40 -fill gray -draw 'text 20,30 "'$i'"' $file
    fi
done


#rack conv$LABEL.h5 -Q XYZ --format '${select}\n' --expandVariables   --script '-c --formatOut - -o foo-${select}.png' -Q FCORE --exec  -Q FETOP --exec  -Q FCELL --exec 
cmd="rack conv$LABEL.h5 --store append=1 -Q FCELL -c  -Q FMAX -c  -Q FETOP -c -Q PROB -c -O conv-cart$LABEL.png"
echo $cmd 
eval $cmd 


convert -frame 1 +append conv$LABEL*-{cappi,max,etop}.png conv$LABEL.png
convert -frame 1 +append conv-polar$LABEL*_{FCELL,FMAX,FETOP}.png conv$LABEL-fuzzy-polar.png
convert -frame 1 +append conv-cart$LABEL*_{FCELL,FMAX,FETOP,PROB}.png  conv$LABEL-fuzzy-cart.png

convert -append conv$LABEL.png  conv$LABEL-fuzzy-cart.png conv$LABEL-fuzzy-polar.png conv$LABEL-all.png
echo xv  conv$LABEL-all.png
#!/bin/bash

if [ $# == 0 ]; then
    echo "Extract and plot information on tasks (elevation angles and times)."
    echo "(c) fmi.fi Markus.Peura"
    echo "args : <volumes>.h5"
    exit 0
fi

GNUPLOT_ARGS=()

COUNTRY=''

rm args-*.gnu

CONF_FMT='SOURCE="${what:source}"\nNOD="${NOD}"\nWMO="${WMO}"\nPLC="${PLC}"\nLAT="${where:lat}"\nLON="${where:lon}"\nHEIGHT="${where:height}"\nPROJDEF="${where:projdef}"\nBBOX="${where:LL_lon},${where:LL_lat},${where:UR_lon},${where:UR_lat}"\n\nDATE="${what:date}"\nTIME="${what:time}"\n'

for i in $*; do
    BASENAME=${i##*/};
    BASENAME=${BASENAME%.*};
    # echo "rack $i  --format '\${what:startdate}-\${what:starttime} \${where:elangle}\n'  --select dataset:/ -o 'data-\${NOD}-\${WMO}.gnu'" ;
    echo $BASENAME
    ## rack $i --formatFile radar-conf.tpl -o 'data-${NOD}-${WMO}.cnf'  --format '${what:startdate}-${what:starttime} ${where:elangle}\n'  --select dataset:/ -o 'data-${NOD}-${WMO}.txt'
    if [ ! -f ${BASENAME}.txt ] || [ ! -f ${BASENAME}.cnf ]; then
	#rack $i --formatFile radar-conf.tpl -o ${BASENAME}.cnf  --format '${what:startdate}-${what:starttime} ${where:elangle}\n'  --select dataset:/ -o ${BASENAME}.txt
	rack $i --format $CONF_FMT -o ${BASENAME}.cnf  --format '${what:startdate}-${what:starttime} ${where:elangle}\n'  --select dataset:/ -o ${BASENAME}.txt
    fi
    export PLC='' NOD='' WMO=''
    source ${BASENAME}.cnf
    # TITLE="$NOD - $PLC" TIMEFMT='%Y%m%d-%H%M%S' YRANGE=0: STYLE=linespoints  OUTFILE=${BASENAME}.png  gnuplot-simple.sh ${BASENAME}.txt
    #  if [ $NOD == '' ]; then
    #  NOD=${NOD:-'WMO'}
    echo ${BASENAME}.txt $NOD >> args-${NOD:0:2}.gnu
    GNUPLOT_ARGS+=( ${BASENAME}.txt $NOD )
done

echo ${GNUPLOT_ARGS[*]}

YRANGE=${YRANGE:-'0:*'}

for i in args-*.gnu; do
    echo "# $i"
    # $COUNTRY
    # TITLE=`${5:2} $DATE $TIME`
    # could use date to format
    TITLE="${i:5:2} $DATE ${TIME:0:4}"
    # YRANGE='0:*'
    YRANGE=$YRANGE TIMEFMT='%Y%m%d-%H%M%S' STYLE=linespoints  TITLE="$TITLE" XTITLE='time [h:m]'  YTITLE='elevation [d]' OUTFILE=plot-${i%.*}.png  gnuplot-simple2.sh `cat $i`
done

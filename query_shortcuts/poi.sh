#!/bin/bash

ARG_1=`echo $QUERY_STRING | awk -F [=,\&] '{ print $1; }'`
ARG_2=`echo $QUERY_STRING | awk -F [=,\&] '{ print $2; }'`
ARG_3=`echo $QUERY_STRING | awk -F [=,\&] '{ print $3; }'`
ARG_4=`echo $QUERY_STRING | awk -F [=,\&] '{ print $4; }'`
ARG_5=`echo $QUERY_STRING | awk -F [=,\&] '{ print $5; }'`
ARG_6=`echo $QUERY_STRING | awk -F [=,\&] '{ print $6; }'`
ARG_7=`echo $QUERY_STRING | awk -F [=,\&] '{ print $7; }'`
ARG_8=`echo $QUERY_STRING | awk -F [=,\&] '{ print $8; }'`

echo -e "\
<osm-script timeout=\"180\" element-limit=\"10000000\"> \
 \
<query type=\"node\"> \
  <bbox-query w=\"$ARG_2\" s=\"$ARG_3\" e=\"$ARG_4\" n=\"$ARG_5\"/> \
  <has-kv k=\"$ARG_6\" v=\"$ARG_7\"/> \
</query> \
<print mode=\"body\"/> \
 \
</osm-script> \
" >/tmp/bbox_req

REQUEST_METHOD=
/home/roland/osm-3s/cgi-bin/interpreter </tmp/bbox_req

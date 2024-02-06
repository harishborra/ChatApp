msg="$1"
tag="$2"
echo $msg | sed -n "s:.*<$tag>\(.*\)</$tag>.*:\1:p"
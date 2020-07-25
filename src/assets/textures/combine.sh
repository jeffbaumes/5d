i=-1
while read line; do

i=$(($i+1))

convert $line +append .tmp$i.png

echo "$i"


done < struct.txt;

files=""

locations="{0..$i}"

for ((x=0; x<=$i; x++)); do
files="${files} .tmp${x}.png"
done

convert $files -append merged.png

rm -f .tmp*
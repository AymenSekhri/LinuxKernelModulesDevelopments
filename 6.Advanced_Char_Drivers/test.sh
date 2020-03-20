for i in $(seq 0 8); do
    sudo ./install.sh
    echo "Driver $i is loaded"
    sleep 1
    sudo ./clean.sh
    echo "Driver $i is unloaded"
done
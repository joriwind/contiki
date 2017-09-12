
case $1 in
    border)
        (cd examples/ipv6/rpl-border-router/ && sudo make connect-router)
        ;;
    node)
        (cd examples/ipv6/rpl-border-router/ && sudo make login MOTES=/dev/ttyUSB1)
        ;;
    docker)
        docker run -it --net=host --privileged -v $(pwd):/shared/ contikibuild-12 /bin/bash
        ;;
    *)
        echo "No device given (border, node)"
        ;;
esac

case $1 in
    border)
        docker run -it --net=host --privileged -v $(pwd):/shared/ contikibuild-12 /bin/bash
        wait 2
        cd shared/examples/ipv6/rpl-border-router/
        make connect-router
        ;;
    node)
        docker run -it --net=host --privileged -v $(pwd):/shared/ contikibuild-12 /bin/bash
        wait 2
        cd shared/examples/ipv6/rpl-border-router/
        make login MOTES=/dev/ttyUSB1
        ;;
    *)
        echo "No device given (border, node)"
        ;;
esac
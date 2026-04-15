#!/bin/sh

if [ -n "$DESTDIR" ] ; then
    case $DESTDIR in
        /*) # ok
            ;;
        *)
            /bin/echo "DESTDIR argument must be absolute... "
            /bin/echo "otherwise python's distutils will bork things."
            exit 1
    esac
fi

echo_and_run() { echo "+ $@" ; "$@" ; }

echo_and_run cd "/home/zhiyuan/moveit/src/moveit/moveit_core"

# ensure that Python install destination exists
echo_and_run mkdir -p "$DESTDIR/home/zhiyuan/moveit/install/lib/python3/dist-packages"

# Note that PYTHONPATH is pulled from the environment to support installing
# into one location when some dependencies were installed in another
# location, #123.
echo_and_run /usr/bin/env \
    PYTHONPATH="/home/zhiyuan/moveit/install/lib/python3/dist-packages:/home/zhiyuan/moveit/build/lib/python3/dist-packages:$PYTHONPATH" \
    CATKIN_BINARY_DIR="/home/zhiyuan/moveit/build" \
    "/usr/bin/python3" \
    "/home/zhiyuan/moveit/src/moveit/moveit_core/setup.py" \
    egg_info --egg-base /home/zhiyuan/moveit/build/moveit/moveit_core \
    build --build-base "/home/zhiyuan/moveit/build/moveit/moveit_core" \
    install \
    --root="${DESTDIR-/}" \
    --install-layout=deb --prefix="/home/zhiyuan/moveit/install" --install-scripts="/home/zhiyuan/moveit/install/bin"

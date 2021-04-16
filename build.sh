USER_PLATFORM="$1"
if [ "$USER_PLATFORM" != linux ] && [ "$USER_PLATFORM" != macos ]
then
    echo "Error: platform must be either linux or macos!" 2>&1
    exit 1
fi

cd "core-$USER_PLATFORM" || exit
bash build.sh

cd .. || exit
cp -r "core-$USER_PLATFORM"/bin/* dist/

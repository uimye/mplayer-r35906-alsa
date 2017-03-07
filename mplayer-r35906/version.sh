#!/bin/sh

test "$1" && extra="-$1"

# releases extract the version number from the VERSION file
version=$(cat VERSION 2> /dev/null)

if test -z $version ; then
# Extract revision number from file used by daily tarball snapshots
# or from the places different Subversion versions have it.
svn_revision=$(cat snapshot_version 2> /dev/null)
test $svn_revision || svn_revision=$(LC_ALL=C svn info 2> /dev/null | grep Revision | cut -d' ' -f2)
test $svn_revision || svn_revision=$(grep revision .svn/entries 2>/dev/null | cut -d '"' -f2)
test $svn_revision || svn_revision=$(sed -n -e '/^dir$/{n;p;q;}' .svn/entries 2>/dev/null)
test $svn_revision && svn_revision=Redxii-SVN-r$svn_revision
test $svn_revision || svn_revision=UNKNOWN
version=$svn_revision
fi

ffmpeg_version=$(cat snapshot_ffmpeg 2> /dev/null)
test $ffmpeg_version || ffmpeg_version=UNKNOWN

NEW_REVISION="#define VERSION \"${version}${extra}\""
OLD_REVISION=$(head -n 1 version.h 2> /dev/null)
FFMPEG_VERSION="#define FFMPEG_VERSION \"${ffmpeg_version}\""
BUILD_DATE="#define BUILD_DATE \"$(date)\""
TITLE='#define MP_TITLE "%s "VERSION" (C) 2000-2013 MPlayer Team\nCustom build by Redxii, http://smplayer.sourceforge.net\nCompiled against FFmpeg version "FFMPEG_VERSION"\nBuild date: "BUILD_DATE"\n\n"'

# Update version.h only on revision changes to avoid spurious rebuilds
if test "$NEW_REVISION" != "$OLD_REVISION"; then
    cat <<EOF > version.h
$NEW_REVISION
$FFMPEG_VERSION
$BUILD_DATE
$TITLE
EOF
fi

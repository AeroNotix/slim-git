# Maintainer: Federico Cinelli <cinelli.federico@gmail.com>

pkgname=slim-git
pkgver=20121218
pkgrel=1
pkgdesc="Desktop-independent graphical login manager for X11. Latest git pull"
arch=('any')
url="http://github.com/AeroNotix/slim-git"
license=('GNU')
depends=('libxmu' 'libpng' 'libjpeg' 'libxft')
makedepends=('git' 'cmake')
conflicts=('slim')
provides=('slim')
source=('slim.service')
md5sums=('a5d6bde9e63899df7d2081e1585bbe54')

_gitroot="git://github.com/AeroNotix/slim-git.git"
_gitname="cower"

build() {
  msg "Connecting to GIT server...."

  if [[ -d $_gitname ]] ; then
    cd "$_gitname" && git pull origin
    msg "The local files are updated."
  else
    git clone "$_gitroot" "$_gitname"
  fi

  msg "GIT checkout done or server timeout"
  msg "Starting make..."

  rm -rf "$srcdir/$_gitname-build"
  cp -r "$srcdir/$_gitname" "$srcdir/$_gitname-build"
  cd "$srcdir/$_gitname-build"
  
  #cmake .. -DUSE_PAM=yes #PAM support
  cmake .
}

package() {
  make -C "$srcdir/$_gitname-build" PREFIX=/usr DESTDIR="$pkgdir" install
}

# vim: ft=sh syn=sh et

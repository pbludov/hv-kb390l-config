Name: hv-kb390l-config
Provides: hv-kb390l-config
Version: 1.1.1
Release: 1%{?dist}
License: LGPL-2.1+
Source: %{name}.tar.gz
URL: https://github.com/pbludov/hv-kb390l-config
Vendor: Pavel Bludov <pbludov@gmail.com>
Packager: Pavel Bludov <pbludov@gmail.com>
Summary: HAVIT KB390L keyboard configuration utility

%description
HAVIT KB390L keyboard unofficial configuration utility.
Allows you to configure the buttons and profiles of your device.

%global debug_package %{nil}

BuildRequires: make, gcc-c++

%{?fedora:BuildRequires:          qt5-qtbase-devel, libusb1-devel,       hidapi-devel}
%{?rhel:BuildRequires:            qt5-qtbase-devel, libusb1-devel,       hidapi-devel}
%{?suse_version:BuildRequires: libqt5-qtbase-devel, libusb-1_0-devel, libhidapi-devel}

%if 0%{?mageia}
%define qmake qmake
BuildRequires: libusb1-devel, hidapi-devel
%ifarch x86_64 amd64
BuildRequires: lib64qt5base5-devel 
%else
BuildRequires: libqt5base5-devel 
%endif
%else
%define qmake qmake-qt5
%endif

%prep
%setup -c %{name}
 
%build
%{qmake} PREFIX=%{_prefix} QMAKE_CFLAGS+="%optflags" QMAKE_CXXFLAGS+="%optflags"
make %{?_smp_mflags}

%install
make install INSTALL_ROOT="%buildroot"

%files
%defattr(-,root,root)
%{_sysconfdir}/udev/rules.d/51-hv-kb390l-keyboard.rules
%{_mandir}/man1/%{name}.1.*
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/48x48/apps/%{name}.png

%posttrans
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
/usr/bin/update-desktop-database &> /dev/null || :

%changelog

* Sat Oct 13 2018 Pavel Bludov <pbludov@gmail.com>
+ Version 1.0
- Initial commit

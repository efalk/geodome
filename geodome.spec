# $Id: geodome.spec,v 1.5 2005/05/15 20:05:24 efalk Exp $

%define	ver	1.2
%define	RELEASE	1
%define rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define	prefix	/usr

Summary: Geodesic dome design program
Name: geodome
Version: %ver
Release: %rel
Group: Applications/Engineering
URL: http://geodome.sourceforge.net
Copyright: MIT
Packager: Edward A. Falk <efalk@sourceforge.net>
Source: http://geodome.sourceforge.net/geodome-%{PACKAGE_VERSION}.tar.gz
Distribution: Redhat 7 and above.
Buildroot: %{_tmppath}/%{name}-root
Docdir: %prefix/doc
Prefix: %prefix

%description
A small suite of programs to help you design a geodesic dome.

Geodome includes a 3-d interactive dome design program, and utilities
to generate 3D Studio files, strut cutting lists, and assembly diagrams.

Requires OpenGL or Mesa

%changelog
    * Sun Nov 7 2004 <efalk@sourceforge.net>
    - First draft

%prep
%setup
make clean

%build
make prefix=%prefix OPT="-O" LDOPTS="-s" all

%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

%doc AUTHORS README INSTRUCTIONS

%{prefix}/bin/dome
%{prefix}/bin/dome_layout
%{prefix}/bin/dome_struts
%{prefix}/bin/dome_cover
%{prefix}/bin/dome_3ds
# %{prefix}/man/man1/xdraft.1

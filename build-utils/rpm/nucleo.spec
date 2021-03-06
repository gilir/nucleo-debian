#%define prefix %{_prefix}
%define prefix /usr

Prefix: %{prefix}
 
%define ver 0.7.3
%define rel 1
%define m_p CFLAGS="-g -O2"

# this is needed on my machine ...
%define c_p --x-includes=/usr/X11R6/include/ --x-libraries=/usr/X11R6/lib/

# Different distributions expect sources to be in different places;
# the following solves this problem, but makes it harder to reuse .src.rpm
%define _sourcedir /tmp

Summary:   nucleo, a toolking for exploring new uses of video   
Name:      nucleo
Version:   %{ver}
Release:   %{rel}
Copyright: GPL
Group:     Development/Tools
Source:    %{name}-%{version}.tar.gz
URL:	   http://insitu.lri.fr/~roussel/projects/nucleo/
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Packager:  nucleo
Autoreq: 1



Provides:  nucleo

Docdir:    %{prefix}/share/doc

%description
nucleo, a toolking for exploring new uses of video

%prep
%setup

%build
./configure --prefix=%{prefix} --mandir=/usr/share/man %{c_p}
make %{m_p}

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT prefix=%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)

%doc AUTHORS COPYING.LESSER  LICENSE NEWS README
%{prefix}/bin/*
%{prefix}/lib/*
%{prefix}/lib/pkgconfig/*
%{prefix}/include/nucleo/*
%{prefix}/share/nucleo/*

%define date%(echo `LC_ALL="C" date +"%a %b %d %Y"`)
%changelog
* Sun Jun 29 2004  Olivier Chapuis
  - First try at making the package

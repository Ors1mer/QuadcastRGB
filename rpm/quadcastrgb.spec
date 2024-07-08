Name:           quadcastrgb
Version:        1.0.4
Release:        2
Summary:        set RGB mode for the microphone HyperX Quadcast S

License:        GPL
URL:            https://ors1mer.xyz/quadcastrgb.html
Source0:        %{name}-%{version}.tgz
ExclusiveArch:	x86_64

BuildRequires:  glibc gcc libusbx-devel
Requires:       glibc libusbx-devel

%description
A minimal RPM build


%build
make quadcastrgb


%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
mkdir -p $RPM_BUILD_ROOT/%{_mandir}
make install BINDIR_INS=$RPM_BUILD_ROOT/%{_bindir} \
             MANDIR_INS=$RPM_BUILD_ROOT/%{_mandir}/man1x/


%files
%{_bindir}/%{name}
%{_mandir}/man1x/%{name}.1.gz


%changelog
* Mon Jul 08 2024 Ors1mer
- Version 1.0.4: a new VID:PID
* Sat Dec 02 2023 Ors1mer
- Version 1.0.3: just a new VID:PID
* Wed May 10 2023 Ors1mer
- Version 1.0.2
* Fri Dec 09 2022 Ors1mer
- Package of the first version


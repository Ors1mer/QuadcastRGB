Name:           quadcastrgb
Version:        1.0.2
Release:        2
Summary:        set RGB mode for the microphone HyperX Quadcast S

License:        GPL
URL:            https://gitlab.com/Ors1mer/QuadcastRGB
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
             MANDIR_INS=$RPM_BUILD_ROOT/%{_mandir}


%files
%{_bindir}/%{name}
%{_mandir}/%{name}.1.gz


%changelog
* Wed May 10 2023 Ors1mer
- Version 1.0.2
* Thu Dec 09 2022 Ors1mer
- Package of the first version

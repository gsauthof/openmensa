%bcond_without srpm


Name:       openmensa-fra-bi
Version:    0.6.0
Release:    2%{?dist}
Summary:    Openmensa parsers for Frankfurt/Bielefeld University canteens
URL:        https://github.com/gsauthof/openmensa
License:    GPLv3+
Source:     https://example.org/openmensa-fra-bi.tar

BuildArch:  noarch

Requires:   python3-html5lib
Requires:   python3-requests

%description
Turn Frankurt/Bielefeld Universety canteen HTML menus into openmensa feeds.

%prep
%if %{with srpm}
%autosetup -n openmensa-fra-bi
%endif

%build

%install
mkdir -p %{buildroot}/usr/bin
cp unibi2openmensa.py %{buildroot}/usr/bin/unibi2openmensa
cp fra2openmensa.py %{buildroot}/usr/bin/fra2openmensa

%check

%files
/usr/bin/unibi2openmensa
/usr/bin/fra2openmensa
%doc README.md


%changelog
* Sat Sep 12 2020 Georg Sauthoff <mail@gms.tf> - 0.5.0-1
- initial packaging


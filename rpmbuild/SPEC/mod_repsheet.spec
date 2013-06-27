Summary: A reputation based intelligence engine for Apache
Name: mod_repsheet
Version: 0.6
Release: 1
License: ASL 2.0
Group: System Environment/Daemons
URL: https://github.com/repsheet/repsheet
Source: http://getrepsheet.com/releases/%{name}-%{version}.zip
Source1: 00_mod_repsheet.conf
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
Requires: httpd hiredis
BuildRequires: httpd-devel pcre-devel hiredis-devel libtool automake autoconf check-devel


%description 
Repsheet is a reputation based intelligence engine for web
applications. It is embedded into the Apache web server and provides
additional protection against attackers.

%prep
%setup -q

%build
./autogen.sh
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/httpd/conf.d/
install -D -m755 src/.libs/mod_repsheet.so $RPM_BUILD_ROOT/%{_libdir}/httpd/modules/mod_repsheet.so
install -D -m644 %{SOURCE1} $RPM_BUILD_ROOT/%{_sysconfdir}/httpd/conf.d/00_mod_repsheet.conf

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc LICENSE
%{_libdir}/httpd/modules/mod_repsheet.so
%config %{_sysconfdir}/httpd/conf.d/00_mod_repsheet.conf

%changelog
* Sat Jun 15 2013 Aaron Bedra <aaron@aaronbedra.com> - 0.4-1
- Initial build.


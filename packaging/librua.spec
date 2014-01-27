Name:           librua
Version:        0.1.0
Release:        34
License:        Apache-2.0
Summary:        Recently used application
Group:          Application Framework/Libraries
Source0:        librua-%{version}.tar.gz
Source1001:     librua.manifest
BuildRequires:  cmake
BuildRequires:  sqlite3
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(libtzplatform-config)

%description
Recently used application library

%package devel
Summary:        Recently used application (devel)
Requires:       %{name} = %{version}

%description devel
Recently used application library (devel)

%prep
%setup -q
cp %{SOURCE1001} .

%build
%cmake .
make %{?_smp_mflags}

%install
%make_install
mkdir -p %{buildroot}%{TZ_SYS_DB}
sqlite3 %{buildroot}%{TZ_SYS_DB}/.rua.db < data/rua_db.sql

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%config(noreplace) %attr(0660,root,%{TZ_SYS_USER_GROUP}) %{TZ_SYS_DB}/.rua.db*
%{_libdir}/librua.so.*
%license LICENSE

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%dir %{_includedir}/rua
%{_includedir}/rua/*.h
%{_libdir}/librua.so
%{_libdir}/pkgconfig/rua.pc


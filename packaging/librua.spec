
Name:       librua
Summary:    Recently used application
Version:    0.1.0
Release:    33
Group:      System/Libraries
License:    Apache License, Version 2.0
Source0:    librua-%{version}.tar.gz
Source1001: packaging/librua.manifest 
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(db-util)


%description
Recently used application library



%package devel
Summary:    Recently used application (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Recently used application library (devel)


%prep
%setup -q -n %{name}-%{version}


%build
cp %{SOURCE1001} .
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}/opt/dbspace/
sqlite3 %{buildroot}/opt/dbspace/.rua.db < %{buildroot}/opt/share/rua_db.sql

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig



%files
%manifest librua.manifest
%defattr(-,root,root,-)
%config(missingok) /opt/share/rua_db.sql
%attr(660,root,app) /opt/dbspace/.rua.db
%attr(660,root,app) /opt/dbspace/.rua.db-journal
/usr/lib/librua.so.*



%files devel
%manifest librua.manifest
%defattr(-,root,root,-)
/usr/include/rua/*.h
/usr/lib/librua.so
/usr/lib/pkgconfig/rua.pc


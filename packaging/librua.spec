
Name:       librua
Summary:    Recently used application
Version:    0.1.0
Release:    33
Group:      System/Libraries
License:    Apache License, Version 2.0
Source0:    librua-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(post): /usr/bin/sqlite3
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
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post
/sbin/ldconfig
mkdir -p /opt/dbspace/
sqlite3 /opt/dbspace/.rua.db < /opt/share/rua_db.sql
rm -rf /opt/share/rua_db.sql
chown 0:5000 /opt/dbspace/.rua.db
chown 0:5000 /opt/dbspace/.rua.db-journal
chmod 660 /opt/dbspace/.rua.db
chmod 660 /opt/dbspace/.rua.db-journal

%postun -p /sbin/ldconfig



%files
%defattr(-,root,root,-)
%config(missingok) /opt/share/rua_db.sql
/usr/lib/librua.so.*



%files devel
%defattr(-,root,root,-)
/usr/include/rua/*.h
/usr/lib/librua.so
/usr/lib/pkgconfig/rua.pc


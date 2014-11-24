# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

$install = <<INSTALL
POSTGRES_VERSION="9.3"
apt-key adv --keyserver keyserver.ubuntu.com --recv-keys B97B0AFCAA1A47F044F244A07FCC7D46ACCC4CF8
echo "deb http://apt.postgresql.org/pub/repos/apt/ trusty-pgdg main" > /etc/apt/sources.list.d/pgdg.list
apt-get -q update
apt-get -y -q install nodejs npm python-software-properties software-properties-common python-pip make postgresql-${POSTGRES_VERSION} postgresql-client-${POSTGRES_VERSION} postgresql-contrib-${POSTGRES_VERSION} postgresql-server-dev-${POSTGRES_VERSION} pgxnclient
apt-get -q clean
sudo ln -s /usr/bin/nodejs /usr/bin/node
cd /vagrant
npm install -q statsd
screen -dmS statsd /vagrant/node_modules/statsd/bin/statsd /vagrant/node_modules/statsd/exampleConfig.js
INSTALL

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.box = "Ubuntu"
  config.vm.provision "shell", inline: $install
end

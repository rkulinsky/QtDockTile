/****************************************************************************
 *  qtdockmanager.cpp
 *
 *  Copyright (c) 2011 by Sidorov Aleksey <gorthauer87@ya.ru>
 *
 ***************************************************************************
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
*****************************************************************************/

#include "qtdockmanager_p.h"
#include "qtdockprovider.h"
#include <QPluginLoader>
#include <QLibraryInfo>
#include <QDir>
#include <QLibrary>
#include <QApplication>
#include <QDebug>

//some sugar
#define _providers \
	foreach (QtDockProvider *provider, usableProviders()) \
	provider \

QtDockManager *QtDockManager::instance()
{
	static QtDockManager pointer;
	return &pointer;
}

QtDockProvider *QtDockManager::currentProvider() const
{
	return usableProviders().first();
}

QtDockProviderList QtDockManager::usableProviders() const
{
	//TODO remove foreach
	QtDockProviderList list;
	foreach (QtDockProvider *provider, m_providers) {
		if (provider->isUsable())
			list.append(provider);
	}
	return list;
}

void QtDockManager::setMenu(QMenu *menu)
{
	m_menu = menu;
	_providers->setMenu(menu);
	emit menuChanged(menu);
}

QMenu *QtDockManager::menu() const
{
	return m_menu.data();
}

void QtDockManager::setBadge(const QString &text)
{
	m_badge = text;
	_providers->setBadge(text);
	emit badgeChanged(text);
}

QString QtDockManager::badge() const
{
	return m_badge;
}

void QtDockManager::setProgress(int percent)
{
	m_percent = percent;
	_providers->setProgress(percent);
	emit progressChanged(percent);
}

int QtDockManager::progress() const
{
	return m_percent;
}

void QtDockManager::alert(bool on)
{
	_providers->alert(on);
}

QVariant QtDockManager::platformInvoke(const QByteArray &method, const QVariant &arguments)
{
	QVariant result(QVariant::Invalid);
	QtDockProvider *provider = currentProvider();
	if (provider)
		result = provider->platformInvoke(method, arguments);
	return result;
}

QtDockManager::QtDockManager()
{
	//resolve plugins
	QStringList plugins;
	QDir dir = QLibraryInfo::location(QLibraryInfo::PluginsPath) + QLatin1String("/docktile");
	foreach (QString filename, dir.entryList(QDir::Files))
		if (QLibrary::isLibrary(filename))
			plugins.append(dir.absolutePath() + '/' + filename);
	dir = qApp->applicationDirPath() + QLatin1String("/plugins/docktile");
	foreach (QString filename, dir.entryList(QDir::Files))
		if (QLibrary::isLibrary(filename))
			plugins.append(dir.absolutePath() + '/' + filename);

	QPluginLoader loader;
	foreach (QString plugin, plugins) {
		loader.setFileName(plugin);
		if (loader.load()) {
			QtDockProvider *provider = qobject_cast<QtDockProvider*>(loader.instance());
			if (provider)
				addProvider(provider);
			else
				qWarning("Unknown interface in plugin %s", qPrintable(plugin));
		} else
			qWarning("Unable to load plugin %s : %s", qPrintable(plugin), qPrintable(loader.errorString()));
	}
}

QtDockManager::~QtDockManager()
{
	qDeleteAll(m_providers);
}

void QtDockManager::addProvider(QtDockProvider *provider)
{
	m_providers.append(provider);
}

void QtDockManager::removeProvider(QtDockProvider *provider)
{
	m_providers.removeAll(provider);
}

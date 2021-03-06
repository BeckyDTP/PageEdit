/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford, Ontario, Canada
**
**  This file is part of PageEdit.
**
**  PageEdit is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  PageEdit is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with PageEdit.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#include <iostream>

#include <QCoreApplication>
#include <QApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QTranslator>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>

#include "MainWindow.h"
#include "Utility.h"
#include "SettingsStore.h"
#include "UILanguage.h"
#include "pageedit_constants.h"
#include "pageedit_exception.h"


#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
// Returns a QIcon with the PageEdit "PE" logo in various sizes
static QIcon GetApplicationIcon()
{
  QIcon app_icon;
  app_icon.addFile(":/icons/app_icon_32.png",  QSize(32, 32));
  app_icon.addFile(":/icons/app_icon_48.png",  QSize(48, 48));
  app_icon.addFile(":/icons/app_icon_128.png", QSize(128, 128));
  app_icon.addFile(":/icons/app_icon_256.png", QSize(256, 256));
  app_icon.addFile(":/icons/app_icon_512.png", QSize(512, 512));
  return app_icon;
}
#endif


// Creates a MainWindow instance depending
// on command line arguments
static MainWindow *GetMainWindow(const QStringList &arguments)
{
    // We use the first argument as the file to load after starting
    QString filepath;
    if (arguments.size() > 1 && Utility::IsFileReadable(arguments.at(1))) {
        filepath = arguments.at(1);
    }
    return new MainWindow(filepath);
}

// fix me program icons


// Application entry point
int main(int argc, char *argv[])
{
    QT_REQUIRE_VERSION(argc, argv, "5.12.3");

    // Set application information for easier use of QSettings classes
    QCoreApplication::setOrganizationName("sigil-ebook");
    QCoreApplication::setOrganizationDomain("sigil-ebook.com");
    QCoreApplication::setApplicationName("pageedit");
    QCoreApplication::setApplicationVersion("0.0.1");
    QCoreApplication::setAttribute(Qt::AA_DisableShaderDiskCache);

    QApplication app(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

    // set up for translations
    SettingsStore settings;

    // Setup the qtbase_ translator and load the translation for the selected language
    QTranslator qtbaseTranslator;
    const QString qm_name_qtbase = QString("qtbase_%1").arg(settings.uiLanguage());
    // Run though all locations and stop once we find and are able to load
    // an appropriate Qt base translation.
    foreach(QString path, UILanguage::GetPossibleTranslationPaths()) {
        if (QDir(path).exists()) {
	    if (qtbaseTranslator.load(qm_name_qtbase, path)) {
	        break;
	    }
        }
    }
    app.installTranslator(&qtbaseTranslator);

    // Setup the PageEdit translator and load the translation for the selected language
    QTranslator pageeditTranslator;
    const QString qm_name = QString("pageedit_%1").arg(settings.uiLanguage());
    // Run though all locations and stop once we find and are able to load
    // an appropriate translation.
    foreach(QString path, UILanguage::GetPossibleTranslationPaths()) {
        if (QDir(path).exists()) {
	    if (pageeditTranslator.load(qm_name, path)) {
	        break;
	    }
        }
    }
    app.installTranslator(&pageeditTranslator);

    // Check for existing qt_styles.qss in Prefs dir and load it if present
    QString qt_stylesheet_path = Utility::DefinePrefsDir() + "/qt_styles.qss";
    QFileInfo QtStylesheetInfo(qt_stylesheet_path);
    if (QtStylesheetInfo.exists() && QtStylesheetInfo.isFile() && QtStylesheetInfo.isReadable()) {
        QString qtstyles = Utility::ReadUnicodeTextFile(qt_stylesheet_path);
        app.setStyleSheet(qtstyles);
    }

    // Qt's setCursorFlashTime(msecs) (or the docs) are broken
    // According to the docs, setting a negative value should disable cursor blinking 
    // but instead just forces it to look for PlatformSpecific Themeable Hints to get 
    // a value which for Mac OS X is hardcoded to 1000 ms
    // This was the only way I could get Qt to disable cursor blinking on a Mac if desired
    if (qEnvironmentVariableIsSet("PAGEEDIT_DISABLE_CURSOR_BLINK")) {
      // qDebug() << "trying to disable text cursor blinking";
      app.setCursorFlashTime(0);
      // qDebug() << "cursorFlashTime: " << app.cursorFlashTime();
    }

// application icons linuxicons
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    app.setWindowIcon(GetApplicationIcon());
#if QT_VERSION >= 0x050700
    // Wayland needs this clarified in order to propery assign the icon 
    app.setDesktopFileName(QStringLiteral("pageedit.desktop"));
#endif
#endif

    const QStringList &arguments = QCoreApplication::arguments();
    MainWindow *widget = GetMainWindow(arguments);
    widget->show();
    return app.exec();
    
}

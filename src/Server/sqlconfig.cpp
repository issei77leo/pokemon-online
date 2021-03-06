#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QMessageBox>

#include "sql.h"
#include "sqlconfig.h"
#include <Utilities/otherwidgets.h>
#include "security.h"
#include "tiermachine.h"

SQLConfigWindow::SQLConfigWindow()
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *v = new QVBoxLayout(this);

    QLabel *desc = new QLabel(tr("<b><span style='color:red'>Don't touch anything if you've no clue what SQL is!</span></b><br /><br />For any change to have effect, you need to restart the server."
                                 "<br />If you change the settings without knowledge of what you are doing, you'll probably end up without any users stored anymore.<br/><br/>SQLite is the "
                                 "only system fully supported by default. PostGreSQL needs an external installation, and you then just have to put the .dlls in that are located in PostGreSQL's bin folder in the server folder. "
                                 "MySQL needs the user to get the right DLLs, the MySQL driver and to install a MySQL database too (it is advised to be on linux to do this as this is far less complicated)."
                                 "<br/> Text is the default format, and the fastest. Text files will be stored in the 'serverdb' folder. If you want to move from SQL to Text, export the database, switch "
                                 "to text and restart the server."));
    desc->setWordWrap(true);
    v->addWidget(desc);

    QSettings s("config", QSettings::IniFormat);

    b = new QComboBox();
    b->addItem("Text");
    b->addItem("SQLite");
    b->addItem("PostGreSQL");
    b->addItem("MySQL");
    v->addLayout(new QSideBySide(new QLabel(tr("SQL Database type: ")), b));
    b->setCurrentIndex(s.value("SQL/Driver").toInt()+1);

    name = new QLineEdit();
    name->setText(s.value("SQL/Database").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("Database name: ")), name));

    schema = new QLineEdit();
    schema->setText(s.value("SQL/DatabaseSchema", "").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("Schema: ")), schema));

    user = new QLineEdit();
    user->setText(s.value("SQL/User").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("User: ")), user));

    pass = new QLineEdit();
    pass->setText(s.value("SQL/Pass").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("Password: ")), pass));

    host = new QLineEdit();
    host->setText(s.value("SQL/Host").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("Host: ")), host));

    port = new QSpinBox();
    port->setRange(0, 65535);
    port->setValue(s.value("SQL/Port").toInt());
    v->addLayout(new QSideBySide(new QLabel(tr("Port: ")), port));

    doVacuum = new QCheckBox("Do VACUUM on start if possible (recommended).");
    doVacuum->setChecked(s.value("SQL/VacuumOnStartup", true).toBool());
    v->addWidget(doVacuum);

    QPushButton *exporting = new QPushButton(tr("&Export"));
    connect(exporting, SIGNAL(clicked()), SLOT(exportDatabase()));

    QPushButton *apply = new QPushButton(tr("&Apply"));
    connect(apply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(apply, SIGNAL(clicked()), SLOT(close()));

    v->addLayout(new QSideBySide(exporting, apply));

    connect(b, SIGNAL(activated(int)), SLOT(changeEnabled()));
    changeEnabled();
}

void SQLConfigWindow::changeEnabled()
{
    bool c = (b->currentIndex()-1 == 0);

    user->setDisabled(c);
    port->setDisabled(c);
    host->setDisabled(c);
    pass->setDisabled(c);
    // Schema for PostgreSQL.
    schema->setEnabled(b->currentIndex()-1 == 1);
    //Text does nothing!
    bool t = (b->currentIndex()-1 == -1);
    name->setDisabled(t);
}

void SQLConfigWindow::apply()
{
    QSettings s("config", QSettings::IniFormat);

    s.setValue("SQL/Driver", b->currentIndex()-1);
    s.setValue("SQL/Database", name->text());
    s.setValue("SQL/Port", port->value());
    s.setValue("SQL/User", user->text());
    s.setValue("SQL/Pass", pass->text());
    s.setValue("SQL/Host", host->text());
    s.setValue("SQL/DatabaseSchema", schema->text());
    s.setValue("SQL/VacuumOnStartup", doVacuum->isChecked());
}

void SQLConfigWindow::exportDatabase()
{
    if (QMessageBox::question(this, "Exporting all the data", "Exporting will create a backup of the database with .txt files, and may hang the server a little."
                              " Are you sure you want to continue?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    SecurityManager::exportDatabase();
    TierMachine::obj()->exportDatabase();

    QMessageBox::information(this, "Database Saved!", "The database was saved successfully in text format (serverdb folder). If you want to import it"
                             " from another server, put those files in the other server's directory and make sure the database it has access to is empty,"
                             " otherwise it won't import. You also have to restart the other server.");
}

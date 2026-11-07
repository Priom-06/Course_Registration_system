#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidgetItem>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initDatabase();
    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->adminTable, &QTableWidget::itemClicked, this, [this](QTableWidgetItem *item){
        int row = item->row();
        ui->adminCodeEdit->setText(ui->adminTable->item(row, 0)->text());
        ui->adminTitleEdit->setText(ui->adminTable->item(row, 1)->text());
        ui->adminCreditEdit->setText(ui->adminTable->item(row, 2)->text());
        ui->adminSecEdit->setText(ui->adminTable->item(row, 3)->text());
        ui->adminSubSecEdit->setText(ui->adminTable->item(row, 4)->text());
    });

    connect(ui->availableTable, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *item){
        if (item->column() == 0) {
            ui->availableTable->blockSignals(true);

            double total = 0.0;
            for (int i = 0; i < ui->availableTable->rowCount(); ++i) {
                QTableWidgetItem *chk = ui->availableTable->item(i, 0);
                if (chk && chk->checkState() == Qt::Checked) {
                    total += ui->availableTable->item(i, 3)->text().toDouble();
                }
            }
            ui->totalCreditLabel->setText(QString("Total Credit: %1 / 22.5").arg(total));

            ui->availableTable->blockSignals(false);
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("courses.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", db.lastError().text());
        return;
    }

    QSqlQuery query;

    bool teacherExists = false;

    QSqlQuery checkQuery(db);

    checkQuery.exec("PRAGMA table_info(courses)");

    while (checkQuery.next()) {
        if (checkQuery.value(1).toString() == "teacher") {
            teacherExists = true;
            break;
        }
    }

    if (!teacherExists) {
        QSqlQuery alterQuery(db);

        if (!alterQuery.exec(
                "ALTER TABLE courses ADD COLUMN teacher TEXT")) {

            QMessageBox::critical(
                this,
                "Database Error",
                alterQuery.lastError().text()
                );
        }
    }
    query.exec("CREATE TABLE IF NOT EXISTS courses ("
               "code TEXT PRIMARY KEY, title TEXT, credit REAL, section TEXT, sub_section TEXT, teacher TEXT)");

    query.exec("CREATE TABLE IF NOT EXISTS requests ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, student_id TEXT, student_name TEXT, course_code TEXT, status TEXT, UNIQUE(student_id, course_code))");

    query.exec("INSERT OR IGNORE INTO courses VALUES ('CSE-1221', 'Computer Programming II', 3.0, 'Female', 'A', 'Dept Teacher')");
    query.exec("INSERT OR IGNORE INTO courses VALUES ('CSE-1222', 'Computer Programming II Lab', 1.5, 'Female', 'A', 'Lab Teacher')");
    query.exec("INSERT OR IGNORE INTO courses VALUES ('CSE-1223', 'Discrete Mathematics', 3.0, 'Female', 'A', 'Math Faculty')");
    query.exec("INSERT OR IGNORE INTO courses VALUES ('CSE-1230', 'Competitive Programming 1', 1.0, 'Female', 'A', 'Trainer')");
    query.exec("INSERT OR IGNORE INTO courses VALUES ('EEE-1221', 'Electronics', 3.0, 'Female', 'A', 'EEE Faculty')");
    query.exec("INSERT OR IGNORE INTO courses VALUES ('EEE-1222', 'Electronics Lab', 1.5, 'Female', 'A', 'EEE Lab Faculty')");
    query.exec("INSERT OR IGNORE INTO courses VALUES ('MATH-1207', 'Mathematics II (Geometry and Differential Equations)', 3.0, 'Female', 'A', 'Math Faculty')");
    query.exec("INSERT OR IGNORE INTO courses VALUES ('PHY-1201', 'Physics II (Electromagnetism, Optics and Modern Physics)', 3.0, 'Female', 'A', 'Physics Faculty')");
    query.exec("INSERT OR IGNORE INTO courses VALUES ('GEED-1201', 'Basic Principles of Islam (''Aqidah + ''Ibadah)', 2.0, 'Female', 'A', 'Islamic Studies')");
    query.exec("INSERT OR IGNORE INTO courses VALUES ('PHY-1204', 'Physics II Lab (Electromagnetism, Optics and Modern Physics)', 1.5, 'Female', 'A', 'Physics Lab')");
}

void MainWindow::on_loginBtn_clicked()
{
    QString userId = ui->userIdEdit->text().trimmed();
    QString password = ui->passwordEdit->text().trimmed();
    QString role = ui->roleComboBox->currentText();

    if (userId.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login Failed", "Please enter User ID and Password!");
        return;
    }

    if (role == "Student") {
        ui->stackedWidget->setCurrentIndex(1);
        ui->studentNameLabel->setText("Name: " + userId);
        loadAvailableCourses();
    } else if (role == "Adviser") {
        ui->stackedWidget->setCurrentIndex(2);

        ui->adviserStudentIdEdit->clear();
        ui->adviserStudentNameLbl->setText("Name: -");
        ui->adviserTable->setRowCount(0);
        ui->adviserTotalCreditLbl->setText("Total Credit Hours: 0");
    }
     else if (role == "Admin") {
        ui->stackedWidget->setCurrentIndex(3);
        loadAdminCourses();
    }
}

void MainWindow::on_logoutBtn1_clicked()
{
    ui->userIdEdit->clear();
    ui->passwordEdit->clear();
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::loadAvailableCourses()
{
    ui->availableTable->blockSignals(true);

    QSqlQuery query("SELECT code, title, credit, section, sub_section FROM courses");
    ui->availableTable->setRowCount(0);

    int row = 0;

    while (query.next()) {
        ui->availableTable->insertRow(row);
        QString code = query.value(0).toString();
        double credit = query.value(2).toDouble();

        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        checkItem->setCheckState(Qt::Unchecked);

        ui->availableTable->setItem(row, 0, checkItem);
        ui->availableTable->setItem(row, 1, new QTableWidgetItem(code));
        ui->availableTable->setItem(row, 2, new QTableWidgetItem(query.value(1).toString()));
        ui->availableTable->setItem(row, 3, new QTableWidgetItem(QString::number(credit)));
        ui->availableTable->setItem(row, 4, new QTableWidgetItem(query.value(3).toString()));
        ui->availableTable->setItem(row, 5, new QTableWidgetItem(query.value(4).toString()));
        ui->availableTable->setItem(row, 6, new QTableWidgetItem("Regular"));
        ui->availableTable->setItem(row, 7, new QTableWidgetItem("Not Selected"));

        row++;
    }

    ui->totalCreditLabel->setText("Total Credit: 0.0 / 22.5");
    ui->availableTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->availableTable->blockSignals(false);
}

void MainWindow::on_studentConfirmBtn_clicked()
{
    QString studentId = ui->studentNameLabel->text().replace("Name: ", "").trimmed();
    if (studentId.isEmpty()) {
        studentId = ui->userIdEdit->text().trimmed();
    }

    int addedCount = 0;

    for (int i = 0; i < ui->availableTable->rowCount(); ++i) {
        if (ui->availableTable->item(i, 0)->checkState() == Qt::Checked) {
            QString courseCode = ui->availableTable->item(i, 1)->text();

            QSqlQuery insertQuery;
            insertQuery.prepare("INSERT OR REPLACE INTO requests (student_id, student_name, course_code, status) "
                                "VALUES (:id, :name, :code, 'PENDING')");
            insertQuery.bindValue(":id", studentId);
            insertQuery.bindValue(":name", studentId);
            insertQuery.bindValue(":code", courseCode);

            if (insertQuery.exec()) {
                addedCount++;
            }
            ui->availableTable->setItem(i, 7, new QTableWidgetItem("PENDING"));
        }
    }

    if (addedCount > 0) {
        QMessageBox::information(this, "Registration Status", QString("%1 Course(s) Confirmed for Student ID: %2! Status: PENDING.").arg(addedCount).arg(studentId));
    } else {
        QMessageBox::warning(this, "Notice", "No courses selected!");
    }
}

void MainWindow::on_studentDropBtn_clicked()
{
    int row = ui->availableTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Warning", "Please select a course to drop!");
        return;
    }

    QString courseCode = ui->availableTable->item(row, 1)->text();
    QString studentId = ui->studentNameLabel->text().replace("Name: ", "").trimmed();

    QSqlQuery query;
    query.prepare("DELETE FROM requests WHERE student_id = :student_id AND course_code = :code");
    query.bindValue(":student_id", studentId);
    query.bindValue(":code", courseCode);

    if (query.exec()) {
        ui->availableTable->item(row, 0)->setCheckState(Qt::Unchecked);
        ui->availableTable->setItem(row, 7, new QTableWidgetItem("Not Selected"));
        QMessageBox::information(this, "Success", "Course unregistered successfully!");
    }
}
void MainWindow::loadAdviserRequests(const QString &studentId)
{
    ui->adviserTable->verticalHeader()->setVisible(false);
    ui->adviserTable->setRowCount(0);

    QString filterId = studentId.trimmed();
    ui->adviserStudentNameLbl->setText("Name: " + filterId);

    if (filterId.isEmpty()) {
        ui->adviserTotalCreditLbl->setText("Total Credit Hours: 0");
        return;
    }

    double totalCredit = 0.0;
    int row = 0;

    QSqlQuery query(db);

    QString sql = "SELECT r.id AS req_id, r.course_code, r.status, "
                  "(SELECT title FROM courses WHERE code = r.course_code LIMIT 1) AS title, "
                  "(SELECT credit FROM courses WHERE code = r.course_code LIMIT 1) AS credit, "
                  "(SELECT section FROM courses WHERE code = r.course_code LIMIT 1) AS section, "
                  "(SELECT teacher FROM courses WHERE code = r.course_code LIMIT 1) AS teacher "
                  "FROM requests r "
                  "WHERE r.student_id = :studentId AND LOWER(r.status) = 'pending'";

    if (!query.prepare(sql)) {
        QMessageBox::critical(this, "Prepare Error", query.lastError().text());
        return;
    }

    query.bindValue(":studentId", filterId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Query Execution Error", query.lastError().text());
        return;
    }

    while (query.next()) {
        ui->adviserTable->insertRow(row);

        ui->adviserTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));

        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        checkItem->setCheckState(Qt::Unchecked);
        checkItem->setData(Qt::UserRole, query.value("req_id"));
        ui->adviserTable->setItem(row, 1, checkItem);

        ui->adviserTable->setItem(row, 2, new QTableWidgetItem(query.value("course_code").toString()));
        ui->adviserTable->setItem(row, 3, new QTableWidgetItem(query.value("title").toString()));

        double cr = query.value("credit").toDouble();
        totalCredit += cr;
        ui->adviserTable->setItem(row, 4, new QTableWidgetItem(QString::number(cr, 'f', 1)));

        ui->adviserTable->setItem(row, 5, new QTableWidgetItem(query.value("section").toString()));
        ui->adviserTable->setItem(row, 6, new QTableWidgetItem(query.value("status").toString()));
        ui->adviserTable->setItem(row, 7, new QTableWidgetItem(query.value("teacher").toString()));

        row++;
    }

    ui->adviserTotalCreditLbl->setText(QString("Total Credit Hours: %1").arg(totalCredit));

    ui->adviserTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    if (row == 0) {
        QMessageBox::information(this, "No Pending Requests", "No pending requests found for Student ID: " + filterId);
    }
}
void MainWindow::on_adviserLoadBtn_clicked()
{
    QString studentId = ui->adviserStudentIdEdit->text().trimmed();

    if (studentId.isEmpty()) {
        QMessageBox::warning(
            this,
            "Warning",
            "Please enter Student ID!"
            );
        return;
    }

    loadAdviserRequests(studentId);
}
void MainWindow::on_adviserAcceptBtn_clicked()
{
    int updatedCount = 0;

    for (int i = 0; i < ui->adviserTable->rowCount(); ++i) {
        if (ui->adviserTable->item(i, 1)->checkState() == Qt::Checked) {
            QString reqId = ui->adviserTable->item(i, 1)->data(Qt::UserRole).toString();

            QSqlQuery query(db);
            query.prepare("UPDATE requests SET status = 'Confirmed' WHERE id = :id AND LOWER(status) = 'pending'");
            query.bindValue(":id", reqId);

            if (query.exec()) {
                updatedCount++;
            }
        }
    }

    if (updatedCount > 0) {
        QMessageBox::information(this, "Success", QString("%1 Selected pending course(s) accepted & confirmed!").arg(updatedCount));
        loadAdviserRequests(ui->adviserStudentIdEdit->text().trimmed());
    } else {
        QMessageBox::warning(this, "Warning", "Please select at least one pending course to accept!");
    }
}
void MainWindow::loadAdminCourses(const QString &filter)
{
    QString sql = "SELECT code, title, credit, section, sub_section FROM courses";
    if (!filter.isEmpty()) {
        sql += " WHERE code LIKE '%" + filter + "%' OR title LIKE '%" + filter + "%'";
    }

    QSqlQuery query(sql);
    ui->adminTable->setRowCount(0);
    int row = 0;
    while (query.next()) {
        ui->adminTable->insertRow(row);
        ui->adminTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->adminTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->adminTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        ui->adminTable->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        ui->adminTable->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
        row++;
    }
    ui->adminTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}

void MainWindow::on_adminAddBtn_clicked()
{
    QString code = ui->adminCodeEdit->text().trimmed();
    QString title = ui->adminTitleEdit->text().trimmed();
    double credit = ui->adminCreditEdit->text().toDouble();
    QString sec = ui->adminSecEdit->text().trimmed();
    QString subSec = ui->adminSubSecEdit->text().trimmed();

    if (code.isEmpty() || title.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Course Code & Title required!");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO courses VALUES (:code, :title, :credit, :sec, :sub, 'Faculty')");
    query.bindValue(":code", code);
    query.bindValue(":title", title);
    query.bindValue(":credit", credit);
    query.bindValue(":sec", sec);
    query.bindValue(":sub", subSec);

    if (query.exec()) {
        QMessageBox::information(this, "Success", "New Course added!");
        loadAdminCourses();
    }
}

void MainWindow::on_adminEditBtn_clicked()
{
    int row = ui->adminTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Warning", "Select a course to Edit!");
        return;
    }

    QString code = ui->adminTable->item(row, 0)->text();
    QString newTitle = ui->adminTitleEdit->text().trimmed();
    QString newCreditStr = ui->adminCreditEdit->text().trimmed();
    QString newSec = ui->adminSecEdit->text().trimmed();
    QString newSubSec = ui->adminSubSecEdit->text().trimmed();

    if (newTitle.isEmpty()) newTitle = ui->adminTable->item(row, 1)->text();
    double newCredit = newCreditStr.isEmpty() ? ui->adminTable->item(row, 2)->text().toDouble() : newCreditStr.toDouble();
    if (newSec.isEmpty()) newSec = ui->adminTable->item(row, 3)->text();
    if (newSubSec.isEmpty()) newSubSec = ui->adminSubSecEdit->text().trimmed();

    QSqlQuery query;
    query.prepare("UPDATE courses SET title = :title, credit = :credit, section = :sec, sub_section = :sub WHERE code = :code");
    query.bindValue(":title", newTitle);
    query.bindValue(":credit", newCredit);
    query.bindValue(":sec", newSec);
    query.bindValue(":sub", newSubSec);
    query.bindValue(":code", code);

    if (query.exec()) {
        QMessageBox::information(this, "Success", "Course updated successfully!");
        ui->adminCodeEdit->clear();
        ui->adminTitleEdit->clear();
        ui->adminCreditEdit->clear();
        ui->adminSecEdit->clear();
        ui->adminSubSecEdit->clear();
        loadAdminCourses();
    }
}

void MainWindow::on_adminDeleteBtn_clicked()
{
    int row = ui->adminTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Warning", "Select a course to Delete!");
        return;
    }

    QString code = ui->adminTable->item(row, 0)->text();
    QSqlQuery query;
    query.prepare("DELETE FROM courses WHERE code = :code");
    query.bindValue(":code", code);

    if (query.exec()) {
        QMessageBox::information(this, "Success", "Course deleted successfully!");
        loadAdminCourses();
    }
}

void MainWindow::on_adminSearchEdit_textChanged(const QString &text)
{
    loadAdminCourses(text.trimmed());
}
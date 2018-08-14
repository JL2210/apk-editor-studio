#include "windows/devicemanager.h"
#include "windows/waitdialog.h"
#include "base/application.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QHeaderView>

DeviceManager::DeviceManager(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Device Manager"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QLabel *caption = new QLabel(tr("Select a device:"));

    deviceList = new QListView(this);
    deviceList->setModel(&deviceModel);
    deviceList->setCurrentIndex(QModelIndex());

    QPushButton *btnRefresh = new QPushButton(tr("Refresh"), this);
    btnRefresh->setIcon(app->loadIcon("refresh.png"));

    QVBoxLayout *listLayout = new QVBoxLayout;
    listLayout->addWidget(caption);
    listLayout->addWidget(deviceList);
    listLayout->addWidget(btnRefresh);

    fieldAlias = new QLineEdit(this);
    fieldAlias->setPlaceholderText(tr("Custom name"));
    labelSerial = new QLabel(this);
    labelProduct = new QLabel(this);
    labelModel = new QLabel(this);
    labelDevice = new QLabel(this);

    QGroupBox *groupInfo = new QGroupBox(tr("Device Information"), this);

    QFormLayout *infoLayout = new QFormLayout(groupInfo);
    infoLayout->addRow(deviceModel.headerData(DevicesModel::DeviceAlias, Qt::Horizontal).toString(), fieldAlias);
    infoLayout->addRow(deviceModel.headerData(DevicesModel::DeviceSerial, Qt::Horizontal).toString(), labelSerial);
    infoLayout->addRow(deviceModel.headerData(DevicesModel::DeviceProduct, Qt::Horizontal).toString(), labelProduct);
    infoLayout->addRow(deviceModel.headerData(DevicesModel::DeviceModel, Qt::Horizontal).toString(), labelModel);
    infoLayout->addRow(deviceModel.headerData(DevicesModel::DeviceDevice, Qt::Horizontal).toString(),labelDevice);

    QHBoxLayout *devicesLayout = new QHBoxLayout;
    devicesLayout->addLayout(listLayout, 2);
    devicesLayout->addWidget(groupInfo, 3);

    dialogButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    QPushButton *btnApply = dialogButtons->button(QDialogButtonBox::Apply);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(devicesLayout);
    layout->addWidget(dialogButtons);

    connect(deviceList->selectionModel(), &QItemSelectionModel::currentChanged, [=](const QModelIndex &index) {
        const Device *device = deviceModel.get(index);
        setCurrentDevice(device);
    });
    connect(fieldAlias, &QLineEdit::textChanged, [=](const QString &alias) {
        QModelIndex index = deviceModel.index(deviceList->currentIndex().row(), DevicesModel::DeviceAlias);
        deviceModel.setData(index, alias);
    });
    connect(btnRefresh, &QPushButton::clicked, this, &DeviceManager::refreshDevices);
    connect(btnApply, &QPushButton::clicked, &deviceModel, &DevicesModel::save);
    connect(dialogButtons, &QDialogButtonBox::accepted, this, &DeviceManager::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected, this, &DeviceManager::reject);
    connect(this, &DeviceManager::accepted, &deviceModel, &DevicesModel::save);

    setCurrentDevice(nullptr);
    show();
    refreshDevices();
}

void DeviceManager::refreshDevices()
{
    WAIT
    deviceModel.refresh();
}

const Device *DeviceManager::getDevice()
{
    setWindowTitle(tr("Install APK"));

    QPushButton *btnInstall = dialogButtons->button(QDialogButtonBox::Ok);
    btnInstall->setText(tr("Install"));
    btnInstall->setIcon(app->loadIcon("device.png"));
    btnInstall->setEnabled(false);

    connect(deviceList->selectionModel(), &QItemSelectionModel::currentChanged, [=](const QModelIndex &index) {
        const Device *device = deviceModel.get(index);
        btnInstall->setEnabled(device);
    });

    if (exec() == QDialog::Accepted) {
        const Device *device = deviceModel.get(deviceList->currentIndex());
        return device;
    }

    return nullptr;
}

bool DeviceManager::setCurrentDevice(const Device *device)
{
    if (device) {
        fieldAlias->setEnabled(true);
        fieldAlias->setText(device->getAlias());
        labelSerial->setText(device->getSerial());
        labelProduct->setText(device->getProductString());
        labelModel->setText(device->getModelString());
        labelDevice->setText(device->getDeviceString());
        return true;
    } else {
        const QChar dash(0x2013);
        fieldAlias->setEnabled(false);
        fieldAlias->setText("");
        labelSerial->setText(dash);
        labelProduct->setText(dash);
        labelModel->setText(dash);
        labelDevice->setText(dash);
        return false;
    }
}
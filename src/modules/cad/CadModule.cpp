#include "CadModule.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

#include <QAction>
#include <QDebug>

CadModule::CadModule(QObject* parent)
    : QObject(parent)
{
}

bool CadModule::initialize()
{
    // 监听导入请求
    connect(EventBus::instance(), &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            if (event == "cad.import.request") {
                importStepFile(data.value("path").toString());
            }
        });

    return true;
}

void CadModule::shutdown()
{
}

QList<QAction*> CadModule::menuActions()
{
    return {};
}

void CadModule::importStepFile(const QString& path)
{
    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("正在导入 STEP 文件: %1").arg(path)}
    });

    // TODO: 使用 OCC STEPControl_Reader 读取 STEP 文件
    // TODO: 遍历 TopoDS_Shape 提取面信息
    // TODO: 转换为 OSG 几何节点并发布到 viewer

    // 占位：发布模型加载完成事件
    DataModel::instance()->setModelPath(path);

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("STEP 模型加载成功: %1").arg(path)}
    });
}

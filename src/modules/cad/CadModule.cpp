#include "CadModule.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include <QAction>
#include <QDebug>
#include <QElapsedTimer>

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
    qDeleteAll(m_analyzers);
    m_analyzers.clear();
    m_faces.clear();
}

QList<QAction*> CadModule::menuActions()
{
    return {};
}

TopoDS_Face CadModule::face(int faceIndex) const
{
    if (faceIndex >= 0 && faceIndex < m_faces.size()) {
        return m_faces[faceIndex];
    }
    return TopoDS_Face();
}

SurfaceAnalyzer* CadModule::analyzerForFace(int faceIndex)
{
    if (m_analyzers.contains(faceIndex)) {
        return m_analyzers[faceIndex];
    }

    if (faceIndex >= 0 && faceIndex < m_faces.size()) {
        auto* analyzer = new SurfaceAnalyzer();
        analyzer->setFace(m_faces[faceIndex]);
        m_analyzers.insert(faceIndex, analyzer);
        return analyzer;
    }
    return nullptr;
}

void CadModule::importStepFile(const QString& path)
{
    auto* bus = EventBus::instance();
    auto* data = DataModel::instance();

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("正在导入 STEP 文件: %1").arg(path)}
    });

    QElapsedTimer timer;
    timer.start();

    // 1. 读取 STEP 文件
    if (!m_reader.load(path)) {
        bus->publish("log.message", {
            {"level", "ERROR"},
            {"message", m_reader.errorString()}
        });
        return;
    }

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("STEP 文件读取完成，耗时 %1 ms").arg(timer.elapsed())}
    });

    // 2. 提取所有面
    m_faces.clear();
    qDeleteAll(m_analyzers);
    m_analyzers.clear();

    for (TopExp_Explorer explorer(m_reader.shape(), TopAbs_FACE);
         explorer.More(); explorer.Next()) {
        m_faces.append(TopoDS::Face(explorer.Current()));
    }

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("提取到 %1 个面").arg(m_faces.size())}
    });

    // 3. 分析曲面属性
    QVector<SurfaceInfo> surfaces = m_reader.analyzeSurfaces();

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("曲面分析完成，共 %1 个面，总耗时 %2 ms")
                        .arg(surfaces.size())
                        .arg(timer.elapsed())}
    });

    // 4. 更新数据模型
    data->setModelPath(path);
    data->setSurfaces(surfaces);

    // 5. 通知 Viewer 模块显示模型（携带 shape 指针，避免再走一次事件往返）
    bus->publish("cad.model.loaded", {
        {"path",      path},
        {"faceCount", m_faces.size()},
        {"shape_ptr", QVariant::fromValue(
            reinterpret_cast<quintptr>(&m_reader.shape()))}
    });

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("STEP 模型加载成功: %1").arg(path)}
    });
}

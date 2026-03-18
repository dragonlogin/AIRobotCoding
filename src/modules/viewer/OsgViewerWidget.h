#pragma once

#include <QOpenGLWidget>
#include <QTimer>
#include <QMap>

#include <osgViewer/Viewer>
#include <osgViewer/GraphicsWindow>
#include <osg/Group>
#include <osg/MatrixTransform>

/**
 * @brief OSG 3D 视图 Widget - 嵌入 Qt 的 OpenSceneGraph 渲染器
 *
 * 职责：
 * - 3D 场景渲染（工件模型、机器人、路径、坐标轴）
 * - 交互操作（旋转、平移、缩放、拾取）
 * - 提供场景节点管理接口供各模块使用
 */
class OsgViewerWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit OsgViewerWidget(QWidget* parent = nullptr);
    ~OsgViewerWidget();

    /// 获取场景根节点（各模块向此节点添加内容）
    osg::Group* sceneRoot() const { return m_sceneRoot.get(); }

    /// 添加/移除场景节点
    void addSceneNode(osg::Node* node, const QString& name);
    void removeSceneNode(const QString& name);

    /// 视图控制
    void setViewFront();
    void setViewBack();
    void setViewLeft();
    void setViewRight();
    void setViewTop();
    void setViewBottom();
    void setViewIsometric();
    void fitAll();

signals:
    void cursorMoved(double x, double y, double z);
    void objectPicked(int faceIndex);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void setupScene();
    void setupCamera();
    void setupLighting();
    void addCoordinateAxes();
    void addGridPlane();

    osg::ref_ptr<osgViewer::Viewer> m_viewer;
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_graphicsWindow;
    osg::ref_ptr<osg::Group> m_sceneRoot;
    osg::ref_ptr<osg::Group> m_modelGroup;       // 工件模型
    osg::ref_ptr<osg::Group> m_robotGroup;        // 机器人模型
    osg::ref_ptr<osg::Group> m_pathGroup;         // 打磨路径
    osg::ref_ptr<osg::Group> m_helperGroup;       // 辅助元素（网格、坐标轴）

    QTimer* m_updateTimer = nullptr;

    // 命名节点映射
    QMap<QString, osg::ref_ptr<osg::Node>> m_namedNodes;
};

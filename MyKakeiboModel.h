class MyKakeiboModel : public QSqlTableModel
{
public:
    using QSqlTableModel::QSqlTableModel;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (role == Qt::ToolTipRole && index.column() == 6) { // 6 = 備考列
            QString biko = QSqlTableModel::data(index, Qt::DisplayRole).toString();
            return biko; // そのままツールチップに
        }
        return QSqlTableModel::data(index, role);
    }
};

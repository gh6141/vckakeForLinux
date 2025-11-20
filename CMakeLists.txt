cmake_minimum_required(VERSION 3.16)

project(vckake VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 の必要モジュール
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Sql Charts LinguistTools)

# 翻訳ファイル
set(TS_FILES vckake_ja_JP.ts)

# プロジェクトのソース
set(PROJECT_SOURCES
    main.cpp
    koza.cpp
    koza.h
    BaseTableWidget.cpp
    BaseTableWidget.h
    KozaWidget.cpp
    KozaWidget.h
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    HimokuWidget.cpp
    HimokuWidget.h
    BikoWidget.cpp
    BikoWidget.h
    ShiharaisakimotoWidget.cpp
    ShiharaisakimotoWidget.h
    ThreeRelationShipsWidget.cpp
    ThreeRelationShipsWidget.h
    kakeibotable.cpp
    kakeibotable.h
    KozaComboWidget.h
    BaseComboWidget.h
    HimokuComboWidget.h
    SiharaiComboWidget.h
    ShiharaisakiComboWidget.h
    BikoComboWidget.h
    KakeiboRowData.h
    BalanceListWidget.cpp
    BalanceListWidget.h
    PeriodSelectDialog.cpp
    PeriodSelectDialog.h
    BalanceTrendWidget.cpp
    BalanceTrendWidget.h
    BikoSearchDialog.cpp
    BikoSearchDialog.h
    ComboRegisterHelper.cpp
    ComboRegisterHelper.h
    ClearOnClickLineEdit.cpp
    ClearOnClickLineEdit.h
    PopularCbx.cpp
    PopularCbx.h
    DraggableButton.h
    DraggableGridWidget.cpp
    DraggableGridWidget.h
    OricoRowData.h
    MyKakeiboModel.h
    Expense.h
    ExpenseLoader.cpp
    ExpenseLoader.h
)

# Qt6 用の実行可能ファイル
qt_add_executable(vckake
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    ${TS_FILES}
    WebApiDialog.h
    WebApiDialog.cpp
)

# 翻訳ファイル
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

# ライブラリリンク
target_link_libraries(vckake PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Charts
)

# Mac/iOS 用バンドル設定
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(vckake PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()

# インストール設定
include(GNUInstallDirs)
install(TARGETS vckake
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 完成
qt_finalize_executable(vckake)

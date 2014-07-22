/**
 * Copyright (C) 2011-2012 Marcel Beck <marcel.beck@mbeck.org>
 * Copyright (C) 2013-1014 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/window/Upload.js")
// require("js/omv/module/admin/service/transmissionbt/util/Format.js")
// require("js/omv/module/admin/service/transmissionbt/torrents/window/AddTorrent.js")
// require("js/omv/module/admin/service/transmissionbt/torrents/window/DeleteTorrent.js")

Ext.define("OMV.module.admin.service.transmissionbt.torrents.TorrentList", {
    extend   : "OMV.workspace.grid.Panel",
    requires : [
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc",
        "OMV.module.admin.service.transmissionbt.util.Format",
        "OMV.module.admin.service.transmissionbt.torrents.window.AddTorrent",
        "OMV.module.admin.service.transmissionbt.torrents.window.DeleteTorrent"
    ],

    autoReload        : true,
    hidePagingToolbar : false,
    hideRefreshButton : false,
    hideEditButton    : true,
    rememberSelected  : true,

    uploadButtonText  : _("Upload"),
    resumeButtonText  : _("Resume"),
    pauseButtonText   : _("Pause"),
    topButtonText     : _("Move to top"),
    upButtonText      : _("Move up"),
    downButtonText    : _("Move down"),
    bottomButtonText  : _("Move to bottom"),
    queueMoveWaitMsg  : _("Changing place in queue for selected item(s)"),

    columns : [{
        header    : _("ID"),
        sortable  : true,
        dataIndex : "id",
        flex      : 1
    },{
        header    : _("Name"),
        sortable  : true,
        dataIndex : "name",
        flex      : 4
    },{
        header    : _("Status"),
        sortable  : true,
        dataIndex : "status",
        flex      : 2,
        renderer  : OMV.module.admin.service.transmissionbt.util.Format.statusRenderer
    },{
        header    : _("Done"),
        sortable  : true,
        dataIndex : "percent_done",
        flex      : 3,
        renderer  : OMV.module.admin.service.transmissionbt.util.Format.doneRenderer
    },{
        header    : _("DL-rate"),
        sortable  : true,
        dataIndex : "download_rate",
        flex      : 1,
        renderer  : OMV.module.admin.service.transmissionbt.util.Format.rateRenderer
    },{
        header    : _("UL-rate"),
        sortable  : true,
        dataIndex : "upload_rate",
        flex      : 1,
        renderer  : OMV.module.admin.service.transmissionbt.util.Format.rateRenderer
    },{
        header    : _("Peers"),
        sortable  : true,
        dataIndex : "peers",
        flex      : 1,
        renderer  : OMV.module.admin.service.transmissionbt.util.Format.peersRenderer
    },{
        header    : _("ETA"),
        sortable  : true,
        dataIndex : "eta",
        flex      : 1,
        renderer  : OMV.module.admin.service.transmissionbt.util.Format.etaRenderer
    },{
        header    : _("Date added"),
        sortable  : true,
        dataIndex : "date_added",
        flex      : 2,
        renderer  : OMV.module.admin.service.transmissionbt.util.Format.timestampRenderer
    },{
        header    : _("Date done"),
        sortable  : true,
        dataIndex : "date_done",
        flex      : 2,
        renderer  : OMV.module.admin.service.transmissionbt.util.Format.timestampRenderer
    },{
        header    : _("Ratio"),
        sortable  : true,
        dataIndex : "upload_ratio",
        flex      : 1,
        renderer  : OMV.module.admin.service.transmissionbt.util.Format.ratioRenderer
    },{
        header    : _("Queue"),
        sortable  : true,
        flex      : 1,
        dataIndex : "queue_position"
    }],

    initComponent : function() {
        var me = this;

        Ext.apply(me, {
            store : Ext.create("OMV.data.Store", {
                autoload   : true,
                remoteSort : false,
                model      : OMV.data.Model.createImplicit({
                    idProperty   : "uuid",
                    totalPoperty : "total",
                    fields       : [
                        { name : "id" },
                        { name : "name" },
                        { name : "status" },
                        { name : "percent_done" },
                        { name : "total_size" },
                        { name : "have_valid" },
                        { name : "download_rate" },
                        { name : "upload_rate" },
                        { name : "connected_peers" },
                        { name : "connected_peers_sending" },
                        { name : "eta" },
                        { name : "date_added" },
                        { name : "date_done" },
                        { name : "upload_ratio" },
                        { name : "queue_position" },
                    ]
                }),
                proxy : {
                    type    : "rpc",
                    rpcData : {
                        service : "TransmissionBt",
                        method  : "getTorrentList"
                    }
                }
            })
        });

        // Initialize context menu
        me.menu = me.getMenu();
        me.callParent(arguments);

        // Set up event listeners
        me.on("itemcontextmenu", me.onItemContextMenu, me);
    },

    onReload : function(id, success, response) {
        var me = this;

        if (success) {
            if (!response)  {
                me.disableReloadAndButtons();
            } else {
                if (me.store !== null)
                    me.store.reload();

                me.toggleAddTorrentButtons(true);
            }
        } else {
            OMV.MessageBox.error(null, response);
            me.disableReloadAndButtons();
        }

    },

    disableReloadAndButtons : function() {
        var me = this;

        me.toggleAddTorrentButtons(false);
        me.store.removeAll();
    },

    doReload : function() {
        var me = this;

        // Run a rpc request to see if we
        // should enable automatic reload
        OMV.Rpc.request({
            scope    : me,
            callback : me.onReload,
            rpcData  : {
                service : "TransmissionBt",
                method  : "serverIsRunning"
            }
        });
    },

    getTopToolbarItems : function() {
        var me = this;
        var items = me.callParent(arguments);

        Ext.Array.insert(items, 1, [{
            id      : me.getId() + "-upload",
            xtype   : "button",
            text    : me.uploadButtonText,
            icon    : "images/upload.png",
            iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
            handler : Ext.Function.bind(me.onUploadButton, me, [ me ]),
            scope   : me
        }]);

        Ext.Array.push(items, [{
            id       : me.getId() + "-resume",
            xtype    : "button",
            text     : me.resumeButtonText,
            icon     : "images/play.png",
            iconCls  : Ext.baseCSSPrefix + "btn-icon-16x16",
            handler  : Ext.Function.bind(me.onResumeButton, me, [ me ]),
            disabled : true,
            scope    : me,
            selectionConfig : {
                minSelection : 1,
                maxSelection : 1,
                enableFn     : function(button, records) {
                    var record = records[0];
                    var status = parseInt(record.get("status"), 10);

                    // 0: Torrent is stopped
                    // 1: Queued to check files
                    // 2: Checking files
                    // 3: Queued to download
                    // 4: Downloading
                    // 5: Queued to seed
                    // 6: Seeding
                    if (status === 0) {
                        return true;
                    }

                    return false;
                },
            }
        },{
            id       : me.getId() + "-pause",
            xtype    : "button",
            text     : me.pauseButtonText,
            icon     : "images/pause.png",
            iconCls  : Ext.baseCSSPrefix + "btn-icon-16x16",
            handler  : Ext.Function.bind(me.onPauseButton, me, [ me ]),
            disabled : true,
            scope    : me,
            selectionConfig : {
                minSelection : 1,
                maxSelection : 1,
                enableFn     : function(button, records) {
                    var record = records[0];
                    var status = parseInt(record.get("status"), 10);

                    // 0: Torrent is stopped
                    // 1: Queued to check files
                    // 2: Checking files
                    // 3: Queued to download
                    // 4: Downloading
                    // 5: Queued to seed
                    // 6: Seeding
                    if (status === 0) {
                        return false;
                    }

                    return true;
                },
            }
        }]);

        return items;
    },

    toggleAddTorrentButtons : function(enable) {
        var me = this;

        addTorrentButton = me.queryById(me.getId() + "-add");
        uploadTorrentButton = me.queryById(me.getId() + "-upload");

        if (enable) {
            addTorrentButton.enable();
            uploadTorrentButton.enable();
        } else {
            addTorrentButton.disable();
            uploadTorrentButton.disable();
        }
    },

    getMenu : function() {
        var me = this;

        return Ext.create("Ext.menu.Menu", {
            items: [{
                id      : me.getId() + "-queue-top",
                text    : me.topButtonText,
                icon    : "images/arrow-up.png",
                iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
                handler : Ext.Function.bind(me.onQueueMoveButton, me, [ "top" ]),
                scope   : me
            },{
                id      : me.getId() + "-queue-up",
                text    : me.upButtonText,
                icon    : "images/arrow-up.png",
                iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
                handler : Ext.Function.bind(me.onQueueMoveButton, me, [ "up" ]),
                scope   : me
            },{
                id      : me.getId() + "-queue-down",
                text    : me.downButtonText,
                icon    : "images/arrow-down.png",
                iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
                handler : Ext.Function.bind(me.onQueueMoveButton, me, [ "down" ]),
                scope   : me
            },{
                id      : me.getId() + "-queue-bottom",
                text    : me.bottomButtonText,
                icon    : "images/arrow-down.png",
                iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
                handler : Ext.Function.bind(me.onQueueMoveButton, me, [ "bottom" ]),
                scope   : me
            }]
        });
    },

    onDestroy : function() {
        var me = this;

        me.menu.destroy();
        me.callParent(arguments);
    },

    onItemContextMenu : function (view, record, item, index, e) {
        var me = this;

        e.stopEvent();
        me.menu.showAt(e.getXY());
    },

    onAddButton : function() {
        var me = this;

        Ext.create("OMV.module.admin.service.transmissionbt.torrents.window.AddTorrent", {
            title     : _("Add torrent"),
            listeners : {
                scope  : me,
                submit : function() {
                    me.doReload();
                }
            }
        }).show();
    },

    onUploadButton : function() {
        var me = this;

        Ext.create("OMV.window.Upload", {
            title   : _("Upload torrent"),
            service : "TransmissionBt",
            method  : "uploadTorrent",
            listeners : {
                success : function () {
                    me.doReload();
                },
                scope : me
            }
        }).show();
    },

    onDeleteButton : function() {
        var me = this;
        var records = me.getSelection();

        Ext.create("OMV.module.admin.service.transmissionbt.torrents.window.DeleteTorrent", {
            listeners : {
                scope  : me,
                submit : function(id, values) {
                    // Add delete_local_data to each item
                    Ext.Array.forEach(records, function(record) {
                        record.delete_local_data = values.delete_local_data;
                    });

                    me.startDeletion(records);
                }
            }
        }).show();
    },

    doDeletion : function(record) {
        var me = this;

        OMV.Rpc.request({
            scope : me,
            callback : me.onDeletion,
            rpcData : {
                service : "TransmissionBt",
                method  : "deleteTorrent",
                params  : {
                    id                : record.get("id"),
                    delete_local_data : record.delete_local_data
                }
            }
        });
    },

    onResumeButton : function() {
        var me = this;
        var record = me.getSelected();

        OMV.Rpc.request({
            scope    : me,
            callback : me.doReload,
            rpcData  : {
                service : "TransmissionBt",
                method  : "resumeTorrent",
                params  : {
                    id : record.get("id")
                }
            }
        });
    },

    onPauseButton : function() {
        var me = this;
        var record = me.getSelected();

        OMV.Rpc.request({
            scope    : me,
            callback : me.doReload,
            rpcData  : {
                service : "TransmissionBt",
                method  : "pauseTorrent",
                params  : {
                    id : record.get("id")
                }
            }
        });
    },

    onQueueMoveButton : function(action) {
        var me = this;
        var records = me.getSelection();

        // Add action to each record
        Ext.Array.forEach(records, function(record) {
            record.action = action;
        });

        me.startQueueMove(records);
    },

    startQueueMove : function(records) {
        var me = this;

        if (records.length <= 0)
            return;

        // Store selected records in a local variable
        me.queueMoveActionInfo = {
            records : records,
            count   : records.length
        };

        // Get first record to be moved
        var record = me.queueMoveActionInfo.records.pop();

        // Display progress dialog
        OMV.MessageBox.progress("", me.queueMoveWaitMsg, "");
        me.updateQueueMoveProgress();

        // Execute move function
        me.doQueueMove(record);
    },

    doQueueMove : function(record) {
        var me = this;

        OMV.Rpc.request({
            scope    : me,
            callback : me.onQueueMove,
            rpcData  : {
                service : "TransmissionBt",
                method  : "queueMoveTorrent",
                params  : {
                    id     : record.get("id"),
                    action : record.action
                }
            }
        });
    },

    onQueueMove : function (id, success, response) {
        var me = this;

        if (!success) {
            // Remove temporary local variables
            delete me.queueMoveActionInfo;

            // Hide progress dialog
            OMV.MessageBox.hide();

            // Display error message
            OMV.MessageBox.error(null, response);
        } else {
            if (me.queueMoveActionInfo.records.length > 0) {
                var record = me.queueMoveActionInfo.records.pop();
                var action = me.queueMoveActionInfo.action;

                // Update progress dialog
                me.updateQueueMoveProgress();

                // Execute move function
                me.doQueueMove(record);
            } else {
                // Remove temporary local variables
                delete me.queueMoveActionInfo;

                // Update and hide progress dialog
                OMV.MessageBox.updateProgress(1, _("100% completed ..."));
                OMV.MessageBox.hide();
                me.doReload();
            }
        }
    },

    updateQueueMoveProgress: function() {
        var me = this;

        // Calculate percentage
        var p = (me.queueMoveActionInfo.count - me.queueMoveActionInfo.records.length) /
        me.queueMoveActionInfo.count;

        // Create message text
        var text = Math.round(100 * p) + _("% completed ...");

        // Update progress dialog
        OMV.MessageBox.updateProgress(p, text);
    }
});

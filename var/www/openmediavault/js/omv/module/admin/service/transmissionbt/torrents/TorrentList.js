/**
 * Copyright (C) 2011-2012 Marcel Beck <marcel.beck@mbeck.org>
 * Copyright (C) 2013-2015 OpenMediaVault Plugin Developers
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
    extend: "OMV.workspace.grid.Panel",
    requires: [
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc",
        "OMV.module.admin.service.transmissionbt.util.Format",
        "OMV.module.admin.service.transmissionbt.torrents.window.AddTorrent",
        "OMV.module.admin.service.transmissionbt.torrents.window.DeleteTorrent"
    ],

    autoReload: true,
    hidePagingToolbar: false,
    hideEditButton: true,
    rememberSelected: true,
    disableLoadMaskOnLoad: true,
    reloadInterval: 5000,

    uploadButtonText: _("Upload"),
    resumeButtonText: _("Resume"),
    pauseButtonText: _("Pause"),
    moveButtonText: _("Move"),
    topButtonText: _("Move to top"),
    upButtonText: _("Move up"),
    downButtonText: _("Move down"),
    bottomButtonText: _("Move to bottom"),
    queueMoveWaitMsg: _("Changing place in queue for selected item(s)"),

    columns: [{
        header: _("ID"),
        sortable: true,
        dataIndex: "id",
        flex: 1
    }, {
        header: _("Name"),
        sortable: true,
        dataIndex: "name",
        flex: 4
    }, {
        header: _("Status"),
        sortable: true,
        dataIndex: "status",
        flex: 2,
        renderer: OMV.module.admin.service.transmissionbt.util.Format.statusRenderer
    }, {
        header: _("Done"),
        sortable: true,
        dataIndex: "percent_done",
        flex: 3,
        renderer: OMV.module.admin.service.transmissionbt.util.Format.doneRenderer
    }, {
        header: _("DL-rate"),
        sortable: true,
        dataIndex: "download_rate",
        flex: 1,
        renderer: OMV.module.admin.service.transmissionbt.util.Format.rateRenderer
    }, {
        header: _("UL-rate"),
        sortable: true,
        dataIndex: "upload_rate",
        flex: 1,
        renderer: OMV.module.admin.service.transmissionbt.util.Format.rateRenderer
    }, {
        header: _("Peers"),
        sortable: true,
        dataIndex: "peers",
        flex: 1,
        renderer: OMV.module.admin.service.transmissionbt.util.Format.peersRenderer
    }, {
        header: _("ETA"),
        sortable: true,
        dataIndex: "eta",
        flex: 1,
        renderer: OMV.module.admin.service.transmissionbt.util.Format.etaRenderer
    }, {
        header: _("Date added"),
        sortable: true,
        dataIndex: "date_added",
        flex: 2,
        renderer: OMV.module.admin.service.transmissionbt.util.Format.timestampRenderer
    }, {
        header: _("Date done"),
        sortable: true,
        dataIndex: "date_done",
        flex: 2,
        renderer: OMV.module.admin.service.transmissionbt.util.Format.timestampRenderer
    }, {
        header: _("Ratio"),
        sortable: true,
        dataIndex: "upload_ratio",
        flex: 1,
        renderer: OMV.module.admin.service.transmissionbt.util.Format.ratioRenderer
    }, {
        header: _("Queue"),
        sortable: true,
        flex: 1,
        dataIndex: "queue_position"
    }],

    initComponent: function() {
        Ext.apply(this, {
            store: Ext.create("OMV.data.Store", {
                autoLoad: true,
                remoteSort: false,
                model: OMV.data.Model.createImplicit({
                    idProperty: "uuid",
                    fields: [{
                        name: "id"
                    }, {
                        name: "name"
                    }, {
                        name: "status"
                    }, {
                        name: "percent_done"
                    }, {
                        name: "total_size"
                    }, {
                        name: "have_valid"
                    }, {
                        name: "download_rate"
                    }, {
                        name: "upload_rate"
                    }, {
                        name: "connected_peers"
                    }, {
                        name: "connected_peers_sending"
                    }, {
                        name: "eta"
                    }, {
                        name: "date_added"
                    }, {
                        name: "date_done"
                    }, {
                        name: "upload_ratio"
                    }, {
                        name: "queue_position"
                    }, ]
                }),
                proxy: {
                    type: "rpc",
                    rpcData: {
                        service: "TransmissionBt",
                        method: "getTorrentList"
                    }
                },
                listeners: {
                    beforeload: this.beforeStoreLoad,
                    scope: this
                }
            })
        });

        this.callParent(arguments);
        this.toggleAddTorrentButtons();
    },

    transmissionIsRunning: false,

    beforeStoreLoad: function() {
        // Doing this here means that the status will be delayed.
        // So we won't know if the server is actually running
        // until the next call.
        this.doCheckIfTransmissionIsRunning();

        this.toggleAddTorrentButtons();

        return this.transmissionIsRunning;
    },

    doCheckIfTransmissionIsRunning: function() {
        OMV.Rpc.request({
            scope: this,
            callback: function(id, success, response) {
                var running = false;

                if (success) {
                    if (response) {
                        running = true;
                    }
                }

                this.transmissionIsRunning = running;
            },
            rpcData: {
                service: "TransmissionBt",
                method: "serverIsRunning"
            }
        });
    },

    toggleAddTorrentButtons: function() {
        var status = this.transmissionIsRunning ? "enable" : "disable";
        var addTorrentButton = this.queryById(this.getId() + "-add");
        var uploadTorrentButton = this.queryById(this.getId() + "-upload");

        addTorrentButton[status]();
        uploadTorrentButton[status]();

        if (!this.transmissionIsRunning) {
            this.store.removeAll();
        }
    },

    getTopToolbarItems: function() {
        var items = this.callParent(arguments);

        Ext.Array.insert(items, 1, [{
            id: this.getId() + "-upload",
            xtype: "button",
            text: this.uploadButtonText,
            icon: "images/upload.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            handler: Ext.Function.bind(this.onUploadButton, this),
            scope: this
        }]);

        Ext.Array.push(items, [{
            id: this.getId() + "-resume",
            xtype: "button",
            text: this.resumeButtonText,
            icon: "images/play.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            handler: Ext.Function.bind(this.onResumeButton, this),
            disabled: true,
            scope: this,
            selectionConfig: {
                minSelections: 1,
                maxSelections: 1,
                enabledFn: this.setStatusButtonEnabled
            },
            action: "resume"
        }, {
            id: this.getId() + "-pause",
            xtype: "button",
            text: this.pauseButtonText,
            icon: "images/pause.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            handler: Ext.Function.bind(this.onPauseButton, this),
            disabled: true,
            scope: this,
            selectionConfig: {
                minSelections: 1,
                maxSelections: 1,
                enabledFn: this.setStatusButtonEnabled
            },
            action: "pause"
        }, {
            id: this.getId() + "-queue",
            xtype: "button",
            text: this.moveButtonText,
            icon: "images/menu.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: true,
            scope: this,
            selectionConfig: {
                minSelections: 1
            },
            menu: [{
                id: this.getId() + "-queue-top",
                text: this.topButtonText,
                icon: "images/arrow-up.png",
                iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
                handler: Ext.Function.bind(this.onQueueMoveButton, this, ["top"]),
                scope: this
            }, {
                id: this.getId() + "-queue-up",
                text: this.upButtonText,
                icon: "images/arrow-up.png",
                iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
                handler: Ext.Function.bind(this.onQueueMoveButton, this, ["up"]),
                scope: this
            }, {
                id: this.getId() + "-queue-down",
                text: this.downButtonText,
                icon: "images/arrow-down.png",
                iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
                handler: Ext.Function.bind(this.onQueueMoveButton, this, ["down"]),
                scope: this
            }, {
                id: this.getId() + "-queue-bottom",
                text: this.bottomButtonText,
                icon: "images/arrow-down.png",
                iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
                handler: Ext.Function.bind(this.onQueueMoveButton, this, ["bottom"]),
                scope: this
            }]
        }]);

        return items;
    },

    setStatusButtonEnabled: function(button, records) {
        if (!this.transmissionIsRunning) {
            return false;
        }

        var record = records[0];
        var status = parseInt(record.get("status"), 10);

        // 0: Torrent is stopped
        // 1: Queued to check files
        // 2: Checking files
        // 3: Queued to download
        // 4: Downloading
        // 5: Queued to seed
        // 6: Seeding
        if (status === 0 && button.action === "resume") {
            return true;
        }

        if (status !== 0 && button.action === "pause") {
            return true;
        }

        return false;
    },

    onAddButton: function() {
        Ext.create("OMV.module.admin.service.transmissionbt.torrents.window.AddTorrent", {
            title: _("Add torrent"),
            listeners: {
                scope: this,
                submit: this.doReload()
            }
        }).show();
    },

    onUploadButton: function() {
        Ext.create("OMV.window.Upload", {
            title: _("Upload torrent"),
            service: "TransmissionBt",
            method: "uploadTorrent",
            listeners: {
                scope: this,
                success: this.doReload()
            }
        }).show();
    },

    onDeleteButton: function() {
        var records = this.getSelection();

        Ext.create("OMV.module.admin.service.transmissionbt.torrents.window.DeleteTorrent", {
            listeners: {
                scope: this,
                submit: function(id, values) {
                    // Add delete_local_data to each item
                    Ext.Array.forEach(records, function(record) {
                        record.delete_local_data = values.delete_local_data;
                    });

                    this.startDeletion(records);
                }
            }
        }).show();
    },

    doDeletion: function(record) {
        OMV.Rpc.request({
            scope: this,
            callback: this.onDeletion,
            rpcData: {
                service: "TransmissionBt",
                method: "deleteTorrent",
                params: {
                    id: record.get("id"),
                    delete_local_data: record.delete_local_data
                }
            }
        });
    },

    onResumeButton: function() {
        var record = this.getSelected();

        OMV.Rpc.request({
            scope: this,
            callback: this.doReload,
            rpcData: {
                service: "TransmissionBt",
                method: "resumeTorrent",
                params: {
                    id: record.get("id")
                }
            }
        });
    },

    onPauseButton: function() {
        var record = this.getSelected();

        OMV.Rpc.request({
            scope: this,
            callback: this.doReload,
            rpcData: {
                service: "TransmissionBt",
                method: "pauseTorrent",
                params: {
                    id: record.get("id")
                }
            }
        });
    },

    onQueueMoveButton: function(action) {
        var records = this.getSelection();

        // Add action to each record.
        Ext.Array.forEach(records, function(record) {
            record.action = action;
        });

        this.startQueueMove(records);
    },

    startQueueMove: function(records) {
        if (records.length <= 0) {
            return;
        }

        // Store selected records in a local variable.
        this.queueMoveActionInfo = {
            records: records,
            count: records.length
        };

        // Get first record to be moved.
        var record = this.queueMoveActionInfo.records.pop();

        // Display progress dialog.
        OMV.MessageBox.progress("", this.queueMoveWaitMsg, "");
        this.updateQueueMoveProgress();

        this.doQueueMove(record);
    },

    doQueueMove: function(record) {
        OMV.Rpc.request({
            scope: this,
            callback: this.onQueueMove,
            rpcData: {
                service: "TransmissionBt",
                method: "queueMoveTorrent",
                params: {
                    id: record.get("id"),
                    action: record.action
                }
            }
        });
    },

    onQueueMove: function(id, success, response) {
        if (!success) {
            // Remove temporary local variables.
            delete this.queueMoveActionInfo;

            // Hide progress dialog.
            OMV.MessageBox.hide();

            // Display error message
            OMV.MessageBox.error(null, response);
        } else {
            if (this.queueMoveActionInfo.records.length > 0) {
                var record = this.queueMoveActionInfo.records.pop();
                var action = this.queueMoveActionInfo.action;

                this.updateQueueMoveProgress();

                this.doQueueMove(record);
            } else {
                // Remove temporary local variables.
                delete this.queueMoveActionInfo;

                // Update and hide progress dialog.
                OMV.MessageBox.updateProgress(1, _("100% completed ..."));
                OMV.MessageBox.hide();
                this.doReload();
            }
        }
    },

    updateQueueMoveProgress: function() {
        var percentage = (this.queueMoveActionInfo.count - this.queueMoveActionInfo.records.length) / this.queueMoveActionInfo.count;
        var text = Math.round(100 * percentage) + _("% completed ...");

        OMV.MessageBox.updateProgress(percentage, text);
    }
});

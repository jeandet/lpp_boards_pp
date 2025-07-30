from SciQLopPlots import SciQLopMultiPlotPanel, PropertiesPanel
from PySide6.QtWidgets import QApplication, QMainWindow, QDockWidget
from PySide6.QtCore import Qt, QThread, Signal
from PySide6.QtGui import QColorConstants
import sys
import os
import platform
import numpy as np
import argparse
from datetime import timedelta

from pylpp_boards_pp import list_pcb_lob, PCB_LOB


if platform.system() == "Linux":
    os.environ["QT_QPA_PLATFORM"] = os.environ.get("SCIQLOP_QT_QPA_PLATFORM", "xcb")
    
    
parser = argparse.ArgumentParser(
                    prog='pyfusion GUI',
                    description='An LPP acquisition boards GUI',
                    epilog='')

parser.add_argument('-b', '--board', default='pcb_lob')
args = parser.parse_args()
    
class PCB_LOB_Reader(QThread):
    update_ch1 = Signal(np.ndarray, np.ndarray)
    update_ch2 = Signal(np.ndarray, np.ndarray)
    update_ch3 = Signal(np.ndarray, np.ndarray)
    update_ch4 = Signal(np.ndarray, np.ndarray)
    update_ch5 = Signal(np.ndarray, np.ndarray)

    def __init__(self, parent=None, n_samples=2**8, latency=255):
        super().__init__(parent)
        self.moveToThread(self)
        try:                      
            self._dev = PCB_LOB(list_pcb_lob()[0], samples_count=n_samples, latency=latency)
            self._dev.timeout = timedelta(microseconds=1)
            self._dev.start()
            self._x_axis = np.arange(n_samples, dtype=np.float64)
        except ImportError:
            print("Failed to open device")

    def run(self):
        while True:                                                                                                  
            data = self._dev.samples
            if data is not None:
                self.update_ch1.emit(self._x_axis, np.ascontiguousarray(data[:,0]) * 1.0)
                self.update_ch2.emit(self._x_axis, np.ascontiguousarray(data[:,1]) * 1.0)
                self.update_ch3.emit(self._x_axis, np.ascontiguousarray(data[:,2]) * 1.0)
                self.update_ch4.emit(self._x_axis, np.ascontiguousarray(data[:,3]) * 1.0)
                self.update_ch5.emit(self._x_axis, np.ascontiguousarray(data[:,4]) * 1.0)
            
            
            

class Plots(SciQLopMultiPlotPanel):
    def __init__(self, parent, n_samples=2**10, latency=255):
        SciQLopMultiPlotPanel.__init__(
            self, parent, synchronize_x=True, synchronize_time=True
        )
        self._graphs = []
        if args.board == 'pcb_lob':
            self._reader = PCB_LOB_Reader(n_samples=n_samples, latency=latency)
        else:
            raise ValueError(f"Unsupported board type: {args.board}")
        for ch in range(5):
            p, g = self.plot(
                np.arange(10) * 1.0,
                np.arange(10) * 1.0,
                labels=[f"ch{ch}"],
                colors=[QColorConstants.Blue],
            )
            self._graphs.append(g)
        self._reader.update_ch1.connect(lambda x, y: self._graphs[0].set_data(x, y))
        self._reader.update_ch2.connect(lambda x, y: self._graphs[1].set_data(x, y))
        self._reader.update_ch3.connect(lambda x, y: self._graphs[2].set_data(x, y))
        self._reader.update_ch4.connect(lambda x, y: self._graphs[3].set_data(x, y))
        self._reader.update_ch5.connect(lambda x, y: self._graphs[4].set_data(x, y))

        self._reader.start()


class MainWindow(QMainWindow):
    def __init__(self, n_samples=2**10, latency=255):
        QMainWindow.__init__(self)
        self.setMouseTracking(True)
        self.plots = Plots(self, n_samples=n_samples, latency=latency)
        self.setCentralWidget(self.plots)
        self.properties_panel = PropertiesPanel(self)
        dock = QDockWidget("Properties panel", self)
        dock.setWidget(self.properties_panel)
        self.addDockWidget(Qt.RightDockWidgetArea, dock)


if __name__ == "__main__":
    QApplication.setAttribute(Qt.AA_UseDesktopOpenGL, True)
    QApplication.setAttribute(Qt.AA_ShareOpenGLContexts, True)
    app = QApplication(sys.argv)
    w = MainWindow(n_samples=2**16, latency=255)
    w.show()
    app.exec()                                                                                                     

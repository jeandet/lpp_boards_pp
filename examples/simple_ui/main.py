from SciQLopPlots import SciQLopMultiPlotPanel
from PySide6.QtWidgets import QApplication, QMainWindow
from PySide6.QtCore import Qt, QThread, Signal
from PySide6.QtGui import QColorConstants
import sys
import os
import platform
import numpy as np
import argparse

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

    def __init__(self, parent=None, n_samples=2**8):
        super().__init__(parent)
        self.moveToThread(self)
        try:                      
            self._dev = PCB_LOB(list_pcb_lob()[0], samples_count=n_samples)
            self._dev.start()
            self._x_axis = np.arange(n_samples, dtype=np.float64)
        except ImportError:
            print("Failed to open device")

    def run(self):
        while True:                                                                                                  
            data = self._dev.samples                                                                                                         
            self.update_ch1.emit(self._x_axis, np.ascontiguousarray(data[:,0]) * 1.0)
            self.update_ch2.emit(self._x_axis, np.ascontiguousarray(data[:,1]) * 1.0)                    
            self.update_ch3.emit(self._x_axis, np.ascontiguousarray(data[:,2]) * 1.0)
            self.update_ch4.emit(self._x_axis, np.ascontiguousarray(data[:,3]) * 1.0)                       
            
            
            

class Plots(SciQLopMultiPlotPanel):
    def __init__(self, parent):
        SciQLopMultiPlotPanel.__init__(
            self, parent, synchronize_x=False, synchronize_time=True
        )
        self._graphs = []
        if args.board == 'pcb_lob':
            self._reader = PCB_LOB_Reader(n_samples=2**16)
        else:
            raise ValueError(f"Unsupported board type: {args.board}")
        for ch in range(4):
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

        self._reader.start()


class MainWindow(QMainWindow):
    def __init__(self):
        QMainWindow.__init__(self)
        self.setMouseTracking(True)
        self.plots = Plots(self)
        self.setCentralWidget(self.plots)


if __name__ == "__main__":
    QApplication.setAttribute(Qt.AA_UseDesktopOpenGL, True)
    QApplication.setAttribute(Qt.AA_ShareOpenGLContexts, True)
    app = QApplication(sys.argv)
    w = MainWindow()
    w.show()
    app.exec()                                                                                                     
import ij.IJ;
import ij.ImagePlus;
import ij.Macro;
import ij.io.OpenDialog;
import ij.plugin.PlugIn;
import ij.process.ByteProcessor;

import java.io.File;
import java.io.IOException;

import uk.ac.mrc.hgu.Wlz.WlzException;
import uk.ac.mrc.hgu.Wlz.WlzFileInputStream;
import uk.ac.mrc.hgu.Wlz.WlzIBox2;
import uk.ac.mrc.hgu.Wlz.WlzIVertex2;
import uk.ac.mrc.hgu.Wlz.WlzObject;

public class Import_Woolz extends ImagePlus implements PlugIn {
	public Import_Woolz() {}

	public Import_Woolz(String path) throws IOException, WlzException {
		open(path);
	}

	public void run(String arg) {
		String path = arg;
		if (path == null || path.equals("")) {
			OpenDialog dialog = new OpenDialog("Import Woolz", "");
			String directory = dialog.getDirectory();
			if (directory == null)
				return; // canceled
			path = directory + dialog.getFileName();
		}

		try {
			open(path);

			if (arg.equals("")) show();
		} catch (Exception e) {
			IJ.handleException(e);
			Macro.abort();
		}
	}

	public void open(String path) throws IOException, WlzException {
		WlzFileInputStream in = new WlzFileInputStream(path);
		WlzObject object = WlzObject.WlzReadObj(in);

		// TODO: support 3D images
		// read in pixels
		WlzIBox2 bounds = WlzObject.WlzBoundingBox2I(object);
		WlzIVertex2 origin = new WlzIVertex2(bounds.xMin, bounds.yMin);
		byte[][][] dstArrayDat = new byte[1][][]; // WlzToUArray1D() would require things not exported to Java
		WlzIVertex2 size = new WlzIVertex2(bounds.xMax - bounds.xMin + 1, bounds.yMax - bounds.yMin + 1);
		WlzIVertex2[] dstSize = new WlzIVertex2[1];
		WlzObject.WlzToUArray2D(dstSize, dstArrayDat, object, origin, size, 0);
		byte[] pixels = new byte[size.vtX * size.vtY];
		for (int j = 0; j < size.vtY; j++)
			System.arraycopy(dstArrayDat[0][j], 0, pixels, j * size.vtX, size.vtX);

		// construct image processor
		setProcessor(new ByteProcessor(size.vtX, size.vtY, pixels));
		setTitle(new File(path).getName());

		// set metadata
		String[] facts = new String[] { "" };
		WlzObject.WlzObjectFacts(object, null, facts, 1);
		StringBuilder builder = new StringBuilder();
		for (String fact : facts)
			builder.append(fact).append('\n');
		setProperty("Info", builder.toString());
	}
}
